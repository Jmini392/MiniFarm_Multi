#pragma once

class Player;

class IInteractable
{
public:
    virtual ~IInteractable() = default;

    virtual bool IsInteractable() const { return false; }

    virtual float GetInteractDistance() const { return 2.5f; }
    virtual void OnInteract(Player* player) = 0;
};
