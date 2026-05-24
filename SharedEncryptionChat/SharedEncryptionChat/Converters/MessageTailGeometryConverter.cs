using Avalonia.Data.Converters;
using Avalonia.Media;
using SharedEncryptionChat.Models;
using System;
using System.Collections.Generic;
using System.Globalization;

namespace SharedEncryptionChat.Converters;

/// <summary>
/// Converts message control tail size to geometry object.
/// </summary>
public class MessageTailGeometryConverter : IMultiValueConverter
{
    /// <inheritdoc/>
    public object? Convert(IList<object?> values, Type targetType, object? parameter, CultureInfo culture)
    {
        // expecting tail size value and message side value
        if (2 != values.Count ||
                values[0] is not double tailSize ||
                values[1] is not MessageSide messageSide)
            return null;

        // return tail geometry proportional to `tailSize`, facing correct direction
        string geometry = MessageSide.Sent == messageSide ?
            $"M {2 * tailSize} 0 L 0 {tailSize} L {2 * tailSize} {2 * tailSize} Z"
            : $"M 0 0 L {2 * tailSize} {tailSize} L 0 {2 * tailSize} Z";
        return Geometry.Parse(geometry);
    }
}
