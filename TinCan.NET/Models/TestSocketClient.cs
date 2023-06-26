using System;
using System.Linq;
using System.Threading;

namespace TinCan.NET.Models;

public class TestSocketClient
{
    public static void Run()
    {
        Postbox postbox;
        Console.Write("Socket URI: ");
        postbox = new Postbox(Console.ReadLine()!);
        postbox.FallbackHandler += (evt, arg) =>
        {
            object[] arr = arg;
            Console.WriteLine($"{evt} {"[" + string.Join(", ", arr.Select(x => x.ToString())) + "]"}");
        };

        CancellationTokenSource stopSource = new CancellationTokenSource();
        Thread postThread = new Thread((stopToken) =>
        {
            CancellationToken ct = (CancellationToken)(stopToken ?? throw new ArgumentNullException(nameof(stopToken)));
            while (true)
            {
                postbox.EventLoop(in ct);
                if (ct.IsCancellationRequested)
                    return;
            }
        });
        postThread.Start(stopSource.Token);

        while (true)
        {
            string line = Console.ReadLine()!;
            if (line == "exit")
                break;
            var split = line.IndexOf(' ');
            postbox.Enqueue(line[..split], line[split..]);
        }
        
        stopSource.Cancel();
        if (postThread.IsAlive)
            postThread.Join();
    }
}