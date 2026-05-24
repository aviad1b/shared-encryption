using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Media;

namespace SharedEncryptionChat.Controls;

/// <summary>
/// Custom Control: Panel for displaying a sent chat message.
/// </summary>
public class MessagePanel : TemplatedControl
{
    /// <summary>
    /// BubbleBrush StyledProperty definition
    /// </summary>
    public static readonly StyledProperty<IBrush?> BubbleBrushProperty =
        AvaloniaProperty.Register<MessagePanel, IBrush?>(nameof(BubbleBrush), null);

    /// <summary>
    /// Gets or sets the BubbleBrush property. This StyledProperty 
    /// indicates the brush used to draw the message bubble.
    /// </summary>
    public IBrush? BubbleBrush
    {
        get => this.GetValue(BubbleBrushProperty);
        set => SetValue(BubbleBrushProperty, value);
    }



    /// <summary>
    /// ShowTail StyledProperty definition.
    /// </summary>
    public static readonly StyledProperty<bool> ShowTailProperty =
        AvaloniaProperty.Register<MessagePanel, bool>(nameof(ShowTail), true);

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
    /// TailSize StyledProperty definition
    /// </summary>
    public static readonly StyledProperty<double> TailSizeProperty =
        AvaloniaProperty.Register<MessagePanel, double>(nameof(TailSize), 5);

    /// <summary>
    /// Gets or sets the TailSize property. This StyledProperty 
    /// indicates the size of the speech bubble's tail (if shown).
    /// </summary>
    public double TailSize
    {
        get => GetValue(TailSizeProperty);
        set => SetValue(TailSizeProperty, value);
    }

    /// <summary>
    /// Content StyledProperty definition.
    /// </summary>
    public static readonly StyledProperty<Control?> ContentProperty =
        AvaloniaProperty.Register<MessagePanel, Control?>(nameof(Content), null);

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
