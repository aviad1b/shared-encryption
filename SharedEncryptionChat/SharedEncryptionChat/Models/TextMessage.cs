using Avalonia.Controls;
using SharedEncryptionChat.ViewModels;
using SharedEncryptionChat.Views;

namespace SharedEncryptionChat.Models;

/// <summary>
/// Represents a text message.
/// </summary>
public class TextMessage : Message
{
    /// <summary>
    /// Message content (text).
    /// </summary>
    public required string Content { get; set; }

    /// <inheritdoc/>
    public override UserControl CreateView()
    {
        return new TextView()
        {
            DataContext = new TextViewModel(Content)
        };
    }
}
