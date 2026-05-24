using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;

namespace SharedEncryptionChat.Controls;

/// <summary>
/// Custom Control: Panel for displaying a sent chat message.
/// </summary>
public class SentMessagePanel : TemplatedControl
{
    /// <summary>
    /// ShowTail StyledProperty definition.
    /// </summary>
    public static readonly StyledProperty<bool> ShowTailProperty =
        AvaloniaProperty.Register<SentMessagePanel, bool>(nameof(ShowTail), true);

    /// <summary>
    /// Gets or sets the ShowTail property. This StyledProperty 
    /// indicates whether or not the message bubble tail is shown.
    /// </summary>
    public bool ShowTail
    {
        get => GetValue(ShowTailProperty);
        set => SetValue(ShowTailProperty, value);
    }

    /// <summary>
    /// Content StyledProperty definition.
    /// </summary>
    public static readonly StyledProperty<Control?> ContentProperty =
        AvaloniaProperty.Register<SentMessagePanel, Control?>(nameof(Content), null);

    /// <summary>
    /// Gets or sets the Content property. This StyledProperty 
    /// indicates the content of the message panel.
    /// </summary>
    public Control? Content
    {
        get => GetValue(ContentProperty);
        set => SetValue(ContentProperty, value);
    }
}
