using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Threading;
using CSZeroMQ;
using CSZeroMQ.Constants;
using MessagePack;

namespace TinCan.NET.Models;

public class Postbox
{
    public Postbox(string uri)
    {
        _sock = new ZMQSocket(SocketType.Pair);
        _sock.SetOption(SocketOptionInt.SendTimeout, 50);
        _sock.SetOption(SocketOptionInt.ReceiveTimeout, 50);
        _sock.SetOption(SocketOptionInt.ConnectTimeout, 100);
        _sock.Connect(uri);

        _toSend = new ConcurrentQueue<byte[]>();
        _recvHandlers = new Dictionary<string, Delegate>();
    }

    public void EventLoop(in CancellationToken stopFlag)
    {
        bool didAnything = false;

        try
        {
            var recvResult = _sock.ReceiveMsg();
            
            if (recvResult != null)
            {
                didAnything = true;
                // unpack data
                var unpacked = MessagePackSerializer.Deserialize<dynamic>(recvResult.Span.ToArray());
                // ensure correct formatting
                if (unpacked.GetType() != typeof(object[]))
                    throw new ApplicationException("Bad format (!Array.isArray(root))");
                if (unpacked.Length != 2)
                    throw new ApplicationException("Bad format (root.length != 2)");
                if (unpacked[1].GetType() != typeof(object[]))
                    throw new ApplicationException("Bad format (!Array.isArray(root[1]))");

                string key = unpacked[0];
                object[] args = unpacked[1];
                // find destination and invoke it
                if (_recvHandlers.TryGetValue(key, out var recvHandler))
                {
                    recvHandler.DynamicInvoke(args);
                }
                else
                {
                    FallbackHandler(key, args);
                }
            }
        }
        catch (Exception e)
        {
            Console.WriteLine(e);
        }

        try
        {
            if (_toSend.TryDequeue(out var sendMsg))
            {
                didAnything = true;
                _sock.Send(sendMsg);
            }
        }
        catch (Exception e)
        {
            Console.WriteLine(e);
        }

        if (!didAnything && !stopFlag.IsCancellationRequested)
        {
            Thread.Yield();
        }
    }

    public void Enqueue(string @event, params object[] args)
    {
        var data = new object[] { @event, args };
        var dataBin = MessagePackSerializer.Serialize(data);
        _toSend.Enqueue(dataBin);
    }

    public void Listen(string @event, Delegate listener)
    {
        _recvHandlers[@event] = listener;
    }

    public void Unlisten(string @event)
    {
        _recvHandlers.Remove(@event);
    }

    internal ZMQSocket Socket => _sock;

    public event Action<string, object[]> FallbackHandler = (evt, arg) => {};

    private ZMQSocket _sock;
    private ConcurrentQueue<byte[]> _toSend;
    private Dictionary<string, Delegate> _recvHandlers;
}