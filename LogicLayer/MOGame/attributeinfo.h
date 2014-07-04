/*
 *  The Mana Server
 *  Copyright (C) 2004 - 2013 The Mana Developers
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

#ifndef ATTRIBUTEINFO_H_
#define ATTRIBUTEINFO_H_

#include <limits>
#include <vector>

/**
 * Stackable types.
 * @todo non-stackable malus layers
 */
enum StackableType
{
    Stackable,
    NonStackable,
    NonStackableBonus
};

/**
 * Attribute augmentation methods.
 */
enum ModifierEffectType
{
    Multiplicative,
    Additive
};

struct AttributeModifier
{
    AttributeModifier(StackableType s, ModifierEffectType effect) :
        stackableType(s),
        effectType(effect)
    {}

    StackableType stackableType;
    ModifierEffectType effectType;
};

struct AttributeInfo
{
#undef max
#undef min
    AttributeInfo(int id, const std::string &name):
        id(id),
        name(name),
        persistent(false),
        minimum(std::numeric_limits<double>::min()),
        maximum(std::numeric_limits<double>::max()),
        modifiable(false)
    {}

    int id;
    std::string name;
    bool persistent;
    double minimum;
    double maximum;
    bool modifiable;
    std::vector<AttributeModifier> modifiers;
};

#endif /* ATTRIBUTEINFO_H_ */
