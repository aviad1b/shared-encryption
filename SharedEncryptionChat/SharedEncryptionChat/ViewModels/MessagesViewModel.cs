using SharedEncryptionChat.Models;
using SharedEncryptionChat.Views;
using System.Collections.ObjectModel;

namespace SharedEncryptionChat.ViewModels;

/// <summary>
/// View model of <see cref="MessagesView"/>.
/// </summary>
public class MessagesViewModel(string[] senders) : ViewModelBase
{
    public string[] Senders { get; } = senders;

    /// <summary>
    /// Messages to display.
    /// </summary>
    public ObservableCollection<Message> Messages { get; } = [];
}
