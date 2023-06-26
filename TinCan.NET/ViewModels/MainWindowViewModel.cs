using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using CommunityToolkit.Mvvm.ComponentModel;
using TinCan.NET.Helpers;
using TinCan.NET.Models;

namespace TinCan.NET.ViewModels;

public partial class MainWindowViewModel : ObservableObject
{
    [ObservableProperty] private sbyte _joyX;
    [ObservableProperty] private sbyte _joyY;
    private Buttons.Mask _buttonMask;

    private static readonly TimeSpan JoyBufferTimeout = TimeSpan.FromMilliseconds(10);
    
    private DateTime _lastJoyUpdate = DateTime.MinValue;

    public uint Value => (uint) _buttonMask | ((uint) (byte) JoyX << 16) | ((uint) (byte) JoyY << 24);

    private void SetButtonMask(Buttons.Mask bit, bool value, [CallerMemberName] string? callerMemberName = null)
    {
        OnPropertyChanging(callerMemberName);
        _buttonMask = (_buttonMask & ~bit) | (value ? bit : 0);
        OnPropertyChanged(callerMemberName);
    }

    public bool BtnDUp
    {
        get => (_buttonMask & Buttons.Mask.DUp) != 0;
        set => SetButtonMask(Buttons.Mask.DUp, value);
    }

    public bool BtnDDown
    {
        get => (_buttonMask & Buttons.Mask.DDown) != 0;
        set => SetButtonMask(Buttons.Mask.DDown, value);
    }

    public bool BtnDLeft
    {
        get => (_buttonMask & Buttons.Mask.DLeft) != 0;
        set => SetButtonMask(Buttons.Mask.DLeft, value);
    }

    public bool BtnDRight
    {
        get => (_buttonMask & Buttons.Mask.DRight) != 0;
        set => SetButtonMask(Buttons.Mask.DRight, value);
    }

    public bool BtnCUp
    {
        get => (_buttonMask & Buttons.Mask.CUp) != 0;
        set => SetButtonMask(Buttons.Mask.CUp, value);
    }

    public bool BtnCDown
    {
        get => (_buttonMask & Buttons.Mask.CDown) != 0;
        set => SetButtonMask(Buttons.Mask.CDown, value);
    }

    public bool BtnCLeft
    {
        get => (_buttonMask & Buttons.Mask.CLeft) != 0;
        set => SetButtonMask(Buttons.Mask.CLeft, value);
    }

    public bool BtnCRight
    {
        get => (_buttonMask & Buttons.Mask.CRight) != 0;
        set => SetButtonMask(Buttons.Mask.CRight, value);
    }

    public bool BtnA
    {
        get => (_buttonMask & Buttons.Mask.A) != 0;
        set => SetButtonMask(Buttons.Mask.A, value);
    }

    public bool BtnB
    {
        get => (_buttonMask & Buttons.Mask.B) != 0;
        set => SetButtonMask(Buttons.Mask.B, value);
    }

    public bool BtnStart
    {
        get => (_buttonMask & Buttons.Mask.Start) != 0;
        set => SetButtonMask(Buttons.Mask.Start, value);
    }

    public bool BtnZ
    {
        get => (_buttonMask & Buttons.Mask.Z) != 0;
        set => SetButtonMask(Buttons.Mask.Z, value);
    }

    public bool BtnL
    {
        get => (_buttonMask & Buttons.Mask.L) != 0;
        set => SetButtonMask(Buttons.Mask.L, value);
    }

    public bool BtnR
    {
        get => (_buttonMask & Buttons.Mask.R) != 0;
        set => SetButtonMask(Buttons.Mask.R, value);
    }
}