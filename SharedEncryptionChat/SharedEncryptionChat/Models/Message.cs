namespace SharedEncryptionChat.Models;

/// <summary>
/// Represents a message.
/// </summary>
public class Message
{
    /// <summary>
    /// Side that the message came from (sent/received).
    /// </summary>
    public required MessageSide Side { get; set; }

    /// <summary>
    /// Message sender (<see langword="null"/> for self).
    /// </summary>
    public required string? Sender { get; set; }
}
