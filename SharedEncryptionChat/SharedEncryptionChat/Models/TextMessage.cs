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
}
