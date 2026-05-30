using Avalonia.Controls;

namespace SharedEncryptionChat.Models;

/// <summary>
/// Represents a message.
/// </summary>
public abstract class Message
{
    /// <summary>
    /// Side that the message came from (sent/received).
    /// </summary>
    public required MessageSide Side { get; set; }

    /// <summary>
    /// Message sender (<see langword="null"/> for self).
    /// </summary>
    public required string? Sender { get; set; }

    /// <summary>
    /// Gets view instance to display message.
    /// </summary>
    /// <returns>View instance to display message.</returns>
    public abstract UserControl CreateView();
}
