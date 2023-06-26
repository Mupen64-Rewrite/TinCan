using System;
using Avalonia;
using Avalonia.Data;
using Avalonia.Input;
using Avalonia.Media;
using Avalonia.Media.Immutable;
using TinCan.NET.Helpers;
using Brushes = Avalonia.Media.Brushes;

namespace TinCan.NET.Controls;

public class Joystick : Avalonia.Controls.Control
{
    private static readonly IPen BlackPen = new ImmutablePen(Colors.Black.ToUInt32(), 1.0);
    private static readonly IPen RedPen = new ImmutablePen(Colors.Red.ToUInt32(), 3.0);

    static Joystick()
    {
        AffectsRender<Joystick>(JoyXProperty, JoyYProperty);
    }

    public static readonly StyledProperty<sbyte> JoyXProperty = AvaloniaProperty.Register<Joystick, sbyte>(
        nameof(JoyX), defaultBindingMode: BindingMode.TwoWay);

    public sbyte JoyX
    {
        get => GetValue(JoyXProperty);
        set => SetValue(JoyXProperty, value);
    }
    
    public static readonly StyledProperty<sbyte> JoyYProperty = AvaloniaProperty.Register<Joystick, sbyte>(
        nameof(JoyY), defaultBindingMode: BindingMode.TwoWay);
    public sbyte JoyY
    {
        get => GetValue(JoyYProperty);
        set => SetValue(JoyYProperty, value);
    }
    

    public override void Render(DrawingContext c)
    {
        Point center = Bounds.Center;
        c.FillRectangle(Brushes.White, Bounds);
        c.DrawEllipse(null, BlackPen, Bounds);
        c.DrawLine(BlackPen, new Point(Bounds.Left, center.Y), new Point(Bounds.Right, center.Y));
        c.DrawLine(BlackPen, new Point(center.X, Bounds.Top), new Point(center.X, Bounds.Bottom));

        Point joyPos = new(
            MathHelpers.Lerp(Bounds.Left, Bounds.Right, ((double) JoyX + 128) / 256),
            MathHelpers.Lerp(Bounds.Bottom, Bounds.Top, ((double) JoyY + 128) / 256));
        
        c.DrawLine(RedPen, center, joyPos);
        c.DrawEllipse(Brushes.Blue, null, joyPos, 5.0, 5.0);
    }

    private void UpdatePosition(Point mousePos)
    {
        // Console.WriteLine($"{mousePos.X}, {mousePos.Y}");
        double normX = mousePos.X / Bounds.Width;
        double normY = mousePos.Y / Bounds.Height;
        
        sbyte nextX = (sbyte) Math.Clamp((int) ((normX - 0.5) * 256), -128, 127);
        sbyte nextY = (sbyte) Math.Clamp((int) ((0.5 - normY) * 256), -128, 127);

        if (nextX is > -8 and < 8) nextX = 0;
        if (nextY is > -8 and < 8) nextY = 0;

        JoyX = nextX;
        JoyY = nextY;
    }

    protected override void OnPointerPressed(PointerPressedEventArgs e)
    {
        base.OnPointerPressed(e);
        PointerPoint ptrPoint = e.GetCurrentPoint(this);
        if (ptrPoint.Properties.IsLeftButtonPressed)
            UpdatePosition(ptrPoint.Position);
    }

    protected override void OnPointerMoved(PointerEventArgs e)
    {
        base.OnPointerMoved(e);
        PointerPoint ptrPoint = e.GetCurrentPoint(this);
        if (ptrPoint.Properties.IsLeftButtonPressed)
            UpdatePosition(ptrPoint.Position);
    }

    protected override void OnPointerReleased(PointerReleasedEventArgs e)
    {
        base.OnPointerReleased(e);
        PointerPoint ptrPoint = e.GetCurrentPoint(this);
        if (ptrPoint.Properties.IsLeftButtonPressed)
            UpdatePosition(ptrPoint.Position);
    }
}