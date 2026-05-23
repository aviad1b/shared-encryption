using System.Globalization;

namespace SharedEncryptionChat.Localization;

/// <summary>
/// Primary localization class: Retreives localized strings from resources.
/// </summary>
public class Localization
{
    /// <summary>
    /// <see cref="SharedEncryptionChat.Localization.LanguageHolder"/> of current language.
    /// </summary>
    public static LanguageHolder LanguageHolder { get; } = new(Language.EnglishUS);

    /// <summary>
    /// Sets current language.
    /// </summary>
    /// <param name="language">New language to set.</param>
    public static void SetLanguage(Language language) => LanguageHolder.Language = language;

    /// <summary>
    /// Gets localized string based on current language.
    /// </summary>
    /// <param name="key">Localized string key.</param>
    /// <returns>Localized string, or <see langword="null"/> if not found.</returns>
    public static string? GetString(string key)
        => LanguageHolder.Language.Resources.GetString(key, CultureInfo.CurrentUICulture);
}
