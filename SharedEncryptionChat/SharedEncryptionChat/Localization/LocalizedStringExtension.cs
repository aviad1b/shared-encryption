using Avalonia.Markup.Xaml;
using System;

namespace SharedEncryptionChat.Localization;

/// <summary>
/// Markup extension of localized string.
/// </summary>
public class LocalizedStringExtension : MarkupExtension
{
    /// <summary>
    /// Localized string's key.
    /// </summary>
    public string? Key { get; set; } = null;

    /// <summary>
    /// Format string to use, <see langword="null"/> to ignore.
    /// </summary>
    public string? Format { get; set; } = null;

    /// <summary>
    /// Constructs a localized string.
    /// </summary>
    public LocalizedStringExtension() { }

    /// <summary>
    /// Constructs a localized string with a given key.
    /// </summary>
    /// <param name="key">Localized string's key.</param>
    public LocalizedStringExtension(string? key) => Key = key;

    /// <summary>
    /// Provides actual string based on localized string key.
    /// </summary>
    /// <param name="serviceProvider">Unused.</param>
    /// <returns>A <see cref="string"/> value based on the localization key <see cref="Key"/>.</returns>
    public override object ProvideValue(IServiceProvider serviceProvider)
    {
        // for empty translation key, return empty string
        if (string.IsNullOrEmpty(Key))
            return string.Empty;

        // for non-empty translation key, try getting by key;
        // if nothing was found, use key itself as a placeholder
        string? localized = Localization.GetString(Key);
        if (string.IsNullOrEmpty(localized))
            return Key;

        // if no format is provided, return localized string as-is;
        // otherwise, apply format
        return string.IsNullOrEmpty(Format) ? localized : string.Format(Format, localized);
    }
}
