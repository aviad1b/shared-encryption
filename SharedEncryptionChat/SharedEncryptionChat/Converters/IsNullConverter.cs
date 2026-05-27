using Avalonia;
using Avalonia.Data.Converters;
using System;
using System.Globalization;

namespace SharedEncryptionChat.Converters;

/// <summary>
/// Converter from <see cref="object?"/> to <see cref="bool"/> based on whether or not is <see langword="null"/>
/// (is <see langword="null"/> -> <see langword="true"/>,
/// isn't <see langword="null"/> -> <see langword="false"/>).
/// </summary>
public class IsNullConverter : IValueConverter
{
    /// <inheritdoc/>
    public object? Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
        => value is null;

    /// <inheritdoc/>
    public object? ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
        => value is bool flag && flag ? null : AvaloniaProperty.UnsetValue;
}
