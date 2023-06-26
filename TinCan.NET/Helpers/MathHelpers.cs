using Avalonia;

namespace TinCan.NET.Helpers;

/// <summary>
/// Provides math utilities not provided by <see cref="System.Math"/> or
/// <see cref="Avalonia.Utilities.MathUtilities"/>.
/// </summary>
public static class MathHelpers
{
    /// <summary>
    /// Linearly interpolates between two values.
    /// </summary>
    /// <param name="x">the first value</param>
    /// <param name="y"></param>
    /// <param name="t">some value, preferably between 0 and 1, but not always</param>
    /// <returns></returns>
    public static double Lerp(double x, double y, double t)
    {
        return x * (1.0 - t) + y * t;
    }

    public static Point Lerp(Point A, Point B, double t)
    {
        return new Point(Lerp(A.X, B.X, t), Lerp(A.Y, B.Y, t));
    }
}