using Avalonia.Data.Converters;
using SharedEncryptionChat.Models;
using System;
using System.Globalization;

namespace SharedEncryptionChat.Converters;

/// <summary>
/// Converts message side (sent/received) to message panel bubble column in grid.
/// </summary>
public class MessageSideToBubbleColumnConverter : IValueConverter
{
    /// <inheritdoc/>
    public object? Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        // expecting message side value
        if (value is not MessageSide messageSide)
            return null;

        return MessageSide.Sent == messageSide ? 1 : 0;
    }

    /// <inheritdoc/>
    public object? ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        // expecting column index in 0..1
        if (value is not int col || col < 0 || col > 1)
            return null;

        return 1 == col ? MessageSide.Sent : MessageSide.Received;
    }
}
