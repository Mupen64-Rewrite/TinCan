using System;
using System.Runtime.InteropServices;

namespace TinCan.NET.Helpers;

public enum MupenError
{
    Success = 0,
    NotInit, /* Function is disallowed before InitMupen64Plus() is called */
    AlreadyInit, /* InitMupen64Plus() was called twice */
    Incompatible, /* API versions between components are incompatible */
    InputAssert, /* Invalid parameters for function call, such as ParamValue=NULL for GetCoreParameter() */
    InputInvalid, /* Invalid input data, such as ParamValue="maybe" for SetCoreParameter() to set a BOOL-type value */
    InputNotFound, /* The input parameter(s) specified a particular item which was not found */
    NoMemory, /* Memory allocation failed */
    Files, /* Error opening, creating, reading, or writing to a file */
    Internal, /* Internal error */
    InvalidState, /* Current program state does not allow operation */
    PluginFail, /* A plugin function returned a fatal error */
    SystemFail, /* A system function call, such as an SDL or file operation, failed */
    Unsupported, /* Function call is not supported (ie, core not built with debugger) */
    WrongType
}

public enum MupenPluginType
{
    Null = 0,
    RSP = 1,
    Graphics,
    Audio,
    Input,
    Core
}

public enum MupenMsgLevel
{
    Error = 1,
    Warning,
    Info,
    Status,
    Verbose
}

[StructLayout(LayoutKind.Explicit)]
public struct Buttons
{
    [Flags]
    public enum Mask : ushort
    {
        DRight = (1 << 0),
        DLeft = (1 << 1),
        DDown = (1 << 2),
        DUp = (1 << 3),
        Start = (1 << 4),
        Z = (1 << 5),
        B = (1 << 6),
        A = (1 << 7),
        L = (1 << 8),
        R = (1 << 9),
        CRight = (1 << 10),
        CLeft = (1 << 11),
        CDown = (1 << 12),
        CUp = (1 << 13),
        Reserved1 = (1 << 14),
        Reserved2 = (1 << 15),
    }

    [FieldOffset(0)] public Mask BtnMask;

    [FieldOffset(2)] public sbyte JoyX;

    [FieldOffset(3)] public sbyte JoyY;

    public uint Value
    {
        get => (uint) BtnMask | ((uint) JoyX << 16) | ((uint) JoyY << 24);
        set
        {
            BtnMask = (Mask) (value & 0xFFFF);
            JoyX = (sbyte) ((value >> 16) & 0xFF);
            JoyY = (sbyte) ((value >> 24) & 0xFF);
        }
    }
}

[StructLayout(LayoutKind.Sequential)]
public struct Control
{
    public enum PluginType
    {
        None = 1,
        Mempak = 2,
        RumblePak = 3,
        TransferPak = 4,
        Raw = 5,
        BioPak = 6
    }

    public enum ControllerType
    {
        Standard = 0,
        VRU = 1
    }

    public int Present;
    public int RawData;
    public PluginType Plugin;
    public ControllerType Type;
}

[StructLayout(LayoutKind.Sequential)]
public unsafe struct ControlInfo
{
    public Control* Controls;
}