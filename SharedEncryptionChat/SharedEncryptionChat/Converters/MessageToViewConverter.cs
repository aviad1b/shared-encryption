using Avalonia.Data.Converters;
using SharedEncryptionChat.Models;
using System;
using System.Globalization;

namespace SharedEncryptionChat.Converters;

/// <summary>
/// Converts message to displaying view.
/// </summary>
public class MessageToViewConverter : IValueConverter
{
    /// <inheritdoc/>
    public object? Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        // expecting a message value
        if (value is not Message msg)
            return null;
        return msg.CreateView();
    }

    /// <inheritdoc/>
    public object? ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        throw new NotSupportedException();
    }
}
