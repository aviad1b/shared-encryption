using Avalonia.Data.Converters;
using Avalonia.Layout;
using SharedEncryptionChat.Models;
using System;
using System.Globalization;

namespace SharedEncryptionChat.Converters;

/// <summary>
/// Converts message side (sent/received) to horizontal alignment of panel.
/// </summary>
public class MessageSideToHorizontalAlignmentConverter : IValueConverter
{
    /// <inheritdoc/>
    public object? Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        // expecting message side value
        if (value is not MessageSide messageSide)
            return null;

        return MessageSide.Sent == messageSide ? HorizontalAlignment.Left
            : HorizontalAlignment.Right;
    }

    /// <inheritdoc/>
    public object? ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        // expecting horizontal alignment value
        if (value is not HorizontalAlignment alignment)
            return null;

        return HorizontalAlignment.Left == alignment ? MessageSide.Sent
            : MessageSide.Received;
    }
}
