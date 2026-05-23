using Avalonia.Markup.Xaml;
using System;

namespace SharedEncryptionChat.Localization;

/// <summary>
/// Markup extension of localized flow direction.
/// </summary>
public class LocalizedFlowDirectionExtension : MarkupExtension
{
    /// <summary>
    /// Provides flow direction based on current language.
    /// </summary>
    /// <param name="serviceProvider">Unused.</param>
    /// <returns>Flow direction based on current language.</returns>
    public override object ProvideValue(IServiceProvider serviceProvider)
        => Localization.LanguageHolder.Language.FlowDirection;
}
