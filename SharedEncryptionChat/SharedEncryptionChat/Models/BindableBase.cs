using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace SharedEncryptionChat.Models;

/// <summary>
/// Base class of bindable models.
/// </summary>
public class BindableBase
{
    /// <summary>
    /// Event raised on property changed.
    /// </summary>
    public event PropertyChangedEventHandler? PropertyChanged;

    /// <summary>
    /// Sets a value to an attached property, and raises <see cref="PropertyChanged"/> event if changed.
    /// </summary>
    /// <typeparam name="T">Property type.</typeparam>
    /// <param name="prop">Member attached to property, by ref.</param>
    /// <param name="value">Value to set.</param>
    /// <param name="propertyName">Property name.</param>
    /// <returns></returns>
    protected bool RaiseAndSetIfChanged<T>(ref T prop, T value, [CallerMemberName] string? propertyName = null)
    {
        if (Equals(prop, value))
            return false;

        prop = value;
        RaisePropertyChanged(propertyName);
        return true;
    }

    /// <summary>
    /// Raises <see cref="PropertyChanged"/> event.
    /// </summary>
    /// <param name="propertyName">Property name.</param>
    protected void RaisePropertyChanged([CallerMemberName] string? propertyName = null)
    {
        if (PropertyChanged is not null)
            PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
    }
}
