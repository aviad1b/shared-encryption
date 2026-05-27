using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Documents;
using Avalonia.Controls.Primitives;
using Avalonia.Media;
using SharedEncryptionChat.Models;

namespace SharedEncryptionChat.Controls;

/// <summary>
/// Custom Control: Panel for displaying a sent chat message.
/// </summary>
public class MessagePanel : TemplatedControl
{
    /// <summary>
    /// MessageSide StyledProperty definition.
    /// </summary>
    public static readonly StyledProperty<MessageSide> MessageSideProperty =
        AvaloniaProperty.Register<MessagePanel, MessageSide>(nameof(MessageSide), MessageSide.Sent);

    /// <summary>
    /// Gets or sets the MessageSide property. This StyledProperty 
    /// indicates the side of the displayed message (sent/received).
    /// </summary>
    public MessageSide MessageSide
    {
        get => GetValue(MessageSideProperty);
        set => SetValue(MessageSideProperty, value);
    }

    /// <summary>
    /// BubbleBrush StyledProperty definition.
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
    /// TailSize StyledProperty definition.
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
    /// Title StyledProperty definition.
    /// </summary>
    public static readonly StyledProperty<string?> TitleProperty =
        AvaloniaProperty.Register<MessagePanel, string?>(nameof(Title), null);

    /// <summary>
    /// Gets or sets the Title property. This StyledProperty 
    /// indicates a title displayed above the message.
    /// </summary>
    public string? Title
    {
        get => GetValue(TitleProperty);
        set => SetValue(TitleProperty, value);
    }

    /// <summary>
    /// TitleFontFamily StyledProperty definition.
    /// </summary>
    public static readonly StyledProperty<FontFamily> TitleFontFamilyProperty =
        AvaloniaProperty.Register<MessagePanel, FontFamily>(nameof(TitleFontFamily), FontFamily.Default);

    /// <summary>
    /// Gets or sets the TitleFontFamily property. This StyledProperty 
    /// indicates font family for message title (if displayed).
    /// </summary>
    public FontFamily TitleFontFamily
    {
        get => GetValue(TitleFontFamilyProperty);
        set => SetValue(TitleFontFamilyProperty, value);
    }

    /// <summary>
    /// TitleFontSize StyledProperty definition.
    /// </summary>
    public static readonly StyledProperty<double> TitleFontSizeProperty =
        AvaloniaProperty.Register<MessagePanel, double>(nameof(TitleFontSize), 8);

    /// <summary>
    /// Gets or sets the TitleFontSize property. This StyledProperty 
    /// indicates font size for message title (if displayed).
    /// </summary>
    public double TitleFontSize
    {
        get => GetValue(TitleFontSizeProperty);
        set => SetValue(TitleFontSizeProperty, value);
    }

    /// <summary>
    /// TitleFontStyle StyledProperty definition.
    /// </summary>
    public static readonly StyledProperty<FontStyle> TitleFontStyleProperty =
        AvaloniaProperty.Register<MessagePanel, FontStyle>(nameof(TitleFontStyle), FontStyle.Normal);

    /// <summary>
    /// Gets or sets the TitleFontStyle property. This StyledProperty 
    /// indicates font style for message title (if displayed).
    /// </summary>
    public FontStyle TitleFontStyle
    {
        get => GetValue(TitleFontStyleProperty);
        set => SetValue(TitleFontStyleProperty, value);
    }

    /// <summary>
    /// TitleFontWeight StyledProperty definition.
    /// </summary>
    public static readonly StyledProperty<FontWeight> TitleFontWeightProperty =
        AvaloniaProperty.Register<MessagePanel, FontWeight>(nameof(TitleFontWeight), FontWeight.Normal);

    /// <summary>
    /// Gets or sets the TitleFontWeight property. This StyledProperty 
    /// indicates font weight for message title (if displayed).
    /// </summary>
    public FontWeight TitleFontWeight
    {
        get => GetValue(TitleFontWeightProperty);
        set => SetValue(TitleFontWeightProperty, value);
    }

    /// <summary>
    /// TitleForeground StyledProperty definition.
    /// </summary>
    public static readonly StyledProperty<IBrush?> TitleForegroundProperty =
        AvaloniaProperty.Register<MessagePanel, IBrush?>(
            nameof(Title),
            TextElement.ForegroundProperty.GetDefaultValue(typeof(TextBlock))
        );

    /// <summary>
    /// Gets or sets the TitleForeground property. This StyledProperty 
    /// indicates text color for message title (if displayed).
    /// </summary>
    public IBrush? TitleForeground
    {
        get => GetValue(TitleForegroundProperty);
        set => SetValue(TitleForegroundProperty, value);
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
