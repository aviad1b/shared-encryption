using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SharedEncryptionChat.Localization;

/// <summary>
/// Holds a language for binding.
/// </summary>
/// <param name="language">Initial language.</param>
public class LanguageHolder(Language language) : Models.BindableBase
{
    private Language _language = language;

    /// <summary>
    /// Currently held language.
    /// </summary>
    public Language Language
    {
        get => _language;
        set => this.RaiseAndSetIfChanged(ref _language, value);
    }
}
