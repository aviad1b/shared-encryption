using SharedEncryptionChat.Models;

namespace SharedEncryptionChat.ViewModels.Dummy;

/// <summary>
/// Dummy <see cref="MessagesViewModel"/> used for design.
/// </summary>
public class DummyMessagesViewModel : MessagesViewModel
{
    public DummyMessagesViewModel() : base(["Avi", "Batya", "Gal"])
    {
        Messages.Add(new TextMessage
        {
            Side = MessageSide.Sent,
            Content = "Hello there"
        });

        Messages.Add(new TextMessage
        {
            Side = MessageSide.Received,
            Sender = "Gal",
            Content = "General Kenobi"
        });

        Messages.Add(new TextMessage
        {
            Side = MessageSide.Received,
            Sender = "Gal",
            Content = "You are a bold one"
        });
    }
}
