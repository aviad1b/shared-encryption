using Avalonia.Media;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Resources;
using System.Text;
using System.Threading.Tasks;

namespace SharedEncryptionChat.Localization;

/// <summary>
/// Holds a language's localization information.
/// </summary>
public class Language
{
    /// <summary>
    /// Resource mnager of language's resources.
    /// </summary>
    public ResourceManager Resources { get; }

    /// <summary>
    /// Matching <see cref="System.Globalization.CultureInfo"/> of language.
    /// </summary>
    public CultureInfo CultureInfo { get; }

    /// <summary>
    /// Language's flow direction.
    /// </summary>
    public FlowDirection FlowDirection =>
        CultureInfo.TextInfo.IsRightToLeft ? FlowDirection.RightToLeft
        : FlowDirection.LeftToRight;

    /// <summary>
    /// Private constructor, constructs language from resources file name and culture name.
    /// </summary>
    /// <param name="resourcesName">Language's resources file name.</param>
    /// <param name="cultureName">Language's culture name.</param>
    private Language(string resourcesName, string cultureName)
    {
        Resources = new(
            $"{nameof(SharedEncryptionChat)}.{nameof(Resources)}.{resourcesName}",
            typeof(Localization).Assembly
        );
        CultureInfo = new(cultureName);
    }

    /// <summary>
    /// Language instance for English (US).
    /// </summary>
    public static Language EnglishUS { get; } = new("en_us", "en-US");
}
