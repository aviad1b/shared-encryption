using Avalonia.Data.Converters;
using Avalonia.Media;
using System;
using System.Collections.Generic;
using System.Globalization;

namespace SharedEncryptionChat.Converters;

/// <summary>
/// Converts sender into title foreground brush.
/// </summary>
public class SenderToForegroundConverter : IMultiValueConverter
{
    /// <inheritdoc/>
    public object? Convert(IList<object?> values, Type targetType, object? parameter, CultureInfo culture)
    {
        // expecting sender value and array of possible senders value
        if (2 != values.Count ||
                values[0] is not string sender ||
                values[1] is not string[] possibleSenders)
            return null;

        // expecting brushes list parameter
        if (parameter is not List<IBrush> foregrounds)
            return null;

        // find index of `sender` in `possibleSenders`
        var index = Array.FindIndex(possibleSenders, s => Equals(s, sender));

        // return brush of this index (`null` if wasn't found)
        if (index < 0)
            return null;
        return foregrounds[index % foregrounds.Count];
    }
}
