using SharedEncryptionChat.Views;

namespace SharedEncryptionChat.ViewModels;

/// <summary>
/// View model of <see cref="TextView"/>.
/// </summary>
public class TextViewModel(string text) : ViewModelBase
{
    /// <summary>
    /// Text message content to display.
    /// </summary>
    public string Text { get; } = text;
}
