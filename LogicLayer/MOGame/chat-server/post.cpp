/*
 *  The Mana Server
 *  Copyright (C) 2008-2010  The Mana World Development Team
 *
 *  This file is part of The Mana Server.
 *
 *  The Mana Server is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana Server is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana Server.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "post.h"

#include "../account-server/character.h"
#include "../common/configuration.h"

Letter::Letter(unsigned type, CharacterData *sender, CharacterData *receiver)
 : mId(0), mType(type), mExpiry(0), mSender(sender), mReceiver(receiver)
{
}

Letter::~Letter()
{
    if (mSender)
        delete mSender;

    if (mReceiver)
        delete mReceiver;
}

void Letter::setExpiry(unsigned long expiry)
{
    mExpiry = expiry;
}

unsigned long Letter::getExpiry() const
{
    return mExpiry;
}

void Letter::addText(const std::string &text)
{
    mContents = text;
}

std::string Letter::getContents() const
{
    return mContents;
}

bool Letter::addAttachment(InventoryItem item)
{
    unsigned max = Configuration::getValue("mail_maxAttachments", 3);
    if (mAttachments.size() > max)
    {
        return false;
    }

    mAttachments.push_back(item);

    return true;
}

CharacterData *Letter::getReceiver() const
{
    return mReceiver;
}

CharacterData *Letter::getSender() const
{
    return mSender;
}

std::vector<InventoryItem> Letter::getAttachments() const
{
    return mAttachments;
}

Post::~Post()
{
    std::vector<Letter*>::iterator itr_end = mLetters.end();
    for (std::vector<Letter*>::iterator itr = mLetters.begin();
         itr != itr_end;
         ++itr)
    {
        delete (*itr);
    }

    mLetters.clear();
}

bool Post::addLetter(Letter *letter)
{
    unsigned max = Configuration::getValue("mail_maxLetters", 10);
    if (mLetters.size() > max)
    {
        return false;
    }

    mLetters.push_back(letter);

    return true;
}

Letter* Post::getLetter(int letter) const
{
    if (letter < 0 || (size_t) letter > mLetters.size())
    {
        return nullptr;
    }
    return mLetters[letter];
}

unsigned Post::getNumberOfLetters() const
{
    return mLetters.size();
}

void PostManager::addLetter(Letter *letter)
{
    std::map<CharacterData*, Post*>::iterator itr =
        mPostBox.find(letter->getReceiver());
    if (itr != mPostBox.end())
    {
        itr->second->addLetter(letter);
    }
    else
    {
        Post *post = new Post();
        post->addLetter(letter);
        mPostBox.insert(
            std::pair<CharacterData*, Post*>(letter->getReceiver(), post)
            );
    }
}

Post *PostManager::getPost(CharacterData *player) const
{
    std::map<CharacterData*, Post*>::const_iterator itr = mPostBox.find(player);
    return (itr == mPostBox.end()) ? nullptr : itr->second;
}

void PostManager::clearPost(CharacterData *player)
{
    std::map<CharacterData*, Post*>::iterator itr =
        mPostBox.find(player);
    if (itr != mPostBox.end())
    {
        delete itr->second;
        mPostBox.erase(itr);
    }
}
