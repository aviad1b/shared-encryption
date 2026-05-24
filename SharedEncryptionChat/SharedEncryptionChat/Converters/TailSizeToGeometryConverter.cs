using Avalonia.Data.Converters;
using Avalonia.Media;
using System;
using System.Globalization;

namespace SharedEncryptionChat.Converters;

/// <summary>
/// Converts message control tail size to geometry object.
/// </summary>
public class TailSizeToGeometryConverter : IValueConverter
{
    /// <inheritdoc/>
    public object? Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        // expecting tail size value
        if (value is not double tailSize)
            return null;

        // return tail geometry proportional to `tailSize`
        return Geometry.Parse(
            $"M {2 * tailSize} 0 L 0 {tailSize} L {2 * tailSize} {2 * tailSize} Z"
        );
    }

    /// <inheritdoc/>
    public object? ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        // not supported
        throw new NotSupportedException();
    }
}
