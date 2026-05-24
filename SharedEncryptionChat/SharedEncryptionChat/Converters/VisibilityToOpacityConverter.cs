using Avalonia.Data.Converters;
using System;
using System.Globalization;

namespace SharedEncryptionChat.Converters;

/// <summary>
/// Converts an IsVisible property value 
/// (<see langword="true"/>/<see langword="false"/>) to an Opacity 
/// property value (<see langword="double"/> in range <c>0</c>..<c>1</c>).
/// </summary>
public class VisibilityToOpacityConverter : IValueConverter
{
    /// <inheritdoc/>
    public object? Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        // expecting IsVisible property value (true/false)
        if (value is not bool isVisible)
            return null;

        // opacity 1 for visible, opacity 0 for invisible
        return isVisible ? 1d : 0d;
    }

    /// <inheritdoc/>
    public object? ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        // expecting Opacity property value (double in range 0..1)
        if (value is not double opacity || opacity < 0 || opacity > 1)
            return null;

        // invisible for opacity 0, visible for any other opacity
        return 0 != opacity;
    }
}
