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

#ifndef POST_H
#define POST_H

#include <map>
#include <string>
#include <vector>

#include "../common/inventorydata.h"

class CharacterData;

class Letter
{
public:
    /**
     * Constructor.
     *
     * Before the letter is stored in the database, the unique Id of the letter
     * is 0.
     * @param type Type of Letter - unused
     * @param sender Pointer to character that sent the letter
     * @param receiver Pointer to character that will receive the letter
     */
    Letter(unsigned type, CharacterData *sender, CharacterData *receiver);

    ~Letter();

    /**
     * Gets the unique Id of the letter.
     */
    unsigned long getId() const
    { return mId; }

    /**
     * Sets the unique Id of the letter used as primary key in the database.
     * @param Id Unique id of the letter.
     */
    void setId(unsigned long Id)
    { mId = Id; }

    /**
     * Gets the type of the letter. (unused)
     */
    unsigned getType() const
    { return mType; }

    /**
     * Set the expiry
     */
    void setExpiry(unsigned long expiry);

    /**
     * Get the expiry
     */
    unsigned long getExpiry() const;

    /**
     * Add text contents of letter
     * This overwrites whatever was there previously
     * @param text The content of the letter to add
     */
    void addText(const std::string &text);

    /**
     * Get the text contents of letter
     * @return String containing the text
     */
    std::string getContents() const;

    /**
     * Add an attachment
     * @param item The attachment to add to the letter
     * @return Returns true if the letter doesnt have too many attachments
     */
    bool addAttachment(InventoryItem item);

    /**
     * Get the character receiving the letter
     * @return Returns the Character who will receive the letter
     */
    CharacterData *getReceiver() const;

    /**
     * Get the character who sent the letter
     * @return Returns the Character who sent the letter
     */
    CharacterData *getSender() const;

    /**
     * Get the attachments
     */
    std::vector<InventoryItem> getAttachments() const;

private:
    unsigned mId;
    unsigned mType;
    unsigned long mExpiry;
    std::string mContents;
    std::vector<InventoryItem> mAttachments;
    CharacterData *mSender;
    CharacterData *mReceiver;
};

class Post
{
public:
    ~Post();

    /**
     * Add letter to post
     * @param letter Letter to add
     * @return Returns true if post isnt full
     */
    bool addLetter(Letter *letter);

    /**
     * Return next letter
     */
    Letter* getLetter(int letter) const;

    /**
     * Return number of letters in post
     * @return Returns the size of mLetters
     */
    unsigned getNumberOfLetters() const;

private:
    std::vector<Letter*> mLetters;
};

class PostManager
{
public:
    /**
     * Add letter to post box
     * @param letter Letter to add
     */
    void addLetter(Letter *letter);

    /**
     * Get post for character
     * @param player Character that is getting post
     * @return Returns the post for that character
     */
    Post *getPost(CharacterData *player) const;

    /**
     * Remove the post for character
     */
    void clearPost(CharacterData *player);

private:
    std::map<CharacterData*, Post*> mPostBox;
};

extern PostManager *postalManager;

#endif
