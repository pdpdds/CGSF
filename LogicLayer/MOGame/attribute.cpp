/*
 *  The Mana Server
 *  Copyright (C) 2004-2010  The Mana World Development Team
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

#include "attribute.h"
#include "being.h"
#include "utils/logger.h"
#include <cassert>

AttributeModifiersEffect::AttributeModifiersEffect(StackableType stackableType,
                                                   ModifierEffectType effectType) :
    mCacheVal(0),
    mMod(effectType == Multiplicative ? 1 : 0),
    mStackableType(stackableType),
    mEffectType(effectType)
{
    assert(effectType == Multiplicative
           || effectType == Additive);
    assert(stackableType == Stackable
           || stackableType == NonStackable
           || stackableType == NonStackableBonus);

    LOG_DEBUG("Layer created with effectType " << effectType
              << " and stackableType " << stackableType << ".");
}

AttributeModifiersEffect::~AttributeModifiersEffect()
{
    // ?
    /*mStates.clear();*/
    LOG_WARN("DELETION of attribute effect!");
}

bool AttributeModifiersEffect::add(unsigned short duration,
                                   double value,
                                   double prevLayerValue,
                                   int level)
{
    LOG_DEBUG("Adding modifier with value " << value <<
              " with a previous layer value of " << prevLayerValue << ". "
              "Current mod at this layer: " << mMod << ".");
    bool ret = false;
    mStates.push_back(new AttributeModifierState(duration, value, level));
    switch (mStackableType) {
    case Stackable:
        switch (mEffectType) {
        case Additive:
            if (value)
            {
                ret = true;
                mMod += value;
                mCacheVal = prevLayerValue + mMod;
            }
            break;
        case Multiplicative:
            if (value != 1)
            {
                ret = true;
                mMod *= value;
                mCacheVal = prevLayerValue * mMod;
            }
            break;
        default:
            LOG_FATAL("Attribute modifiers effect: unhandled type '"
                      << mEffectType << "' as a stackable!");
            assert(0);
            break;
        }
        break;
    case NonStackable:
        switch (mEffectType) {
        case Additive:
            if (value > mMod)
            {
                ret = true;
                mMod = value;
                if (mMod > prevLayerValue)
                    mCacheVal = mMod;
            }
            break;
        default:
            LOG_FATAL("Attribute modifiers effect: unhandled type '"
                      << mEffectType << "' as a non-stackable!");
            assert(0);
        }
        // A multiplicative type would also be nonsensical
        break;
    case NonStackableBonus:
        switch (mEffectType) {
        case Additive:
        case Multiplicative:
            if (value > mMod)
            {
                ret = true;
                mMod = value;
                mCacheVal = mEffectType == Additive ? prevLayerValue + mMod
                                              : prevLayerValue * mMod;
            }
            break;
        default:
            LOG_FATAL("Attribute modifiers effect: unhandled type '"
                      << mEffectType << "' as a non-stackable bonus!");
            assert(0);
        }
        break;
    default:
        LOG_FATAL("Attribute modifiers effect: unknown stackable type '"
                  << mStackableType << "'!");
        assert(0);
    }
    return ret;
}

bool durationCompare(const AttributeModifierState *lhs,
                     const AttributeModifierState *rhs)
{
    return lhs->mDuration < rhs->mDuration;
}

bool AttributeModifiersEffect::remove(double value, unsigned id,
                                      bool fullCheck)
{
    /* We need to find and check this entry exists, and erase the entry
       from the list too. */
    if (!fullCheck)
        mStates.sort(durationCompare); /* Search only through those with a duration of 0. */
    bool ret = false;

    for (std::list< AttributeModifierState * >::iterator it = mStates.begin();
         it != mStates.end() && (fullCheck || !(*it)->mDuration);)
    {
        /* Check for a match */
        if ((*it)->mValue != value || (*it)->mId != id)
        {
            ++it;
            continue;
        }

        delete *it;
        mStates.erase(it++);

        /* If this is stackable, we need to update for every modifier affected */
        if (mStackableType == Stackable)
            updateMod(value);

        ret = true;
        if (!id)
            break;
    }
    /*
     * Non stackables only need to be updated once, since this is recomputed
     * from scratch. This is done at the end after modifications have been
     * made as necessary.
     */
    if (ret && mStackableType != Stackable)
        updateMod();
    return ret;
}

void AttributeModifiersEffect::updateMod(double value)
{
    if (mStackableType == Stackable)
    {
        if (mEffectType == Additive)
        {
            mMod -= value;
        }
        else if (mEffectType == Multiplicative)
        {
            if (value)
                mMod /= value;
            else
            {
                mMod = 1;
                for (std::list< AttributeModifierState * >::const_iterator
                     it = mStates.begin(),
                     it_end = mStates.end();
                    it != it_end;
                    ++it)
                    mMod *= (*it)->mValue;
            }
        }
        else LOG_ERROR("Attribute modifiers effect: unhandled type '"
                       << mEffectType << "' as a stackable in cache update!");
    }
    else if (mStackableType == NonStackable || mStackableType == NonStackableBonus)
    {
        if (mMod == value)
        {
            mMod = 0;
            for (std::list< AttributeModifierState * >::const_iterator
                 it = mStates.begin(),
                 it_end = mStates.end();
                it != it_end;
                ++it)
                if ((*it)->mValue > mMod)
                    mMod = (*it)->mValue;
        }
    }
    else
    {
        LOG_ERROR("Attribute modifiers effect: unknown stackable type '"
                  << mStackableType << "' in cache update!");
    }
}

bool AttributeModifiersEffect::recalculateModifiedValue(double newPrevLayerValue)
{
    double oldValue = mCacheVal;
    switch (mEffectType) {
        case Additive:
            switch (mStackableType) {
            case Stackable:
            case NonStackableBonus:
                mCacheVal = newPrevLayerValue + mMod;
            break;
            case NonStackable:
                mCacheVal = newPrevLayerValue < mMod ? mMod : newPrevLayerValue;
            break;
            default:
            LOG_FATAL("Unknown effect type '" << mEffectType << "'!");
            assert(0);
        } break;
        case Multiplicative:
            mCacheVal = mStackableType == Stackable ? newPrevLayerValue * mMod : newPrevLayerValue * mMod;
        break;
        default:
        LOG_FATAL("Unknown effect type '" << mEffectType << "'!");
        assert(0);
    }
    return oldValue != mCacheVal;
}


bool Attribute::add(unsigned short duration, double value,
                    unsigned layer, int id)
{
    assert(mMods.size() > layer);
    LOG_DEBUG("Adding modifier to attribute with duration " << duration
              << ", value " << value
              << ", at layer " << layer
              << " with id " << id);
    if (mMods.at(layer)->add(duration, value,
                            (layer ? mMods.at(layer - 1)->getCachedModifiedValue()
                                   : mBase)
                            , id))
    {
        while (++layer < mMods.size())
        {
            if (!mMods.at(layer)->recalculateModifiedValue(
                       mMods.at(layer - 1)->getCachedModifiedValue()))
            {
                LOG_DEBUG("Modifier added, but modified value not changed.");
                return false;
            }
        }
        LOG_DEBUG("Modifier added. Base value: " << mBase << ", new modified "
                  "value: " << getModifiedAttribute() << ".");
        return true;
    }
    LOG_DEBUG("Failed to add modifier!");
    return false;
}

bool Attribute::remove(double value, unsigned layer,
                       int lvl, bool fullcheck)
{
    assert(mMods.size() > layer);
    if (mMods.at(layer)->remove(value, lvl, fullcheck))
    {
        for (; layer < mMods.size(); ++layer)
            if (!mMods.at(layer)->recalculateModifiedValue(
                        layer ? mMods.at(layer - 1)->getCachedModifiedValue()
                              : mBase))
               return false;
        return true;
    }
    return false;
}

bool AttributeModifiersEffect::tick()
{
    bool ret = false;
    std::list<AttributeModifierState *>::iterator it = mStates.begin();
    while (it != mStates.end())
    {
        if ((*it)->tick())
        {
            double value = (*it)->mValue;
            LOG_DEBUG("Modifier of value " << value << " expiring!");
            delete *it;
            mStates.erase(it++);
            updateMod(value);
            ret = true;
        }
        ++it;
    }
    return ret;
}

Attribute::Attribute(const AttributeInfo *info):
    mBase(0),
    mMinValue(info->minimum),
    mMaxValue(info->maximum)
{
    const std::vector<AttributeModifier> &modifiers = info->modifiers;
    LOG_DEBUG("Construction of new attribute with '" << modifiers.size()
        << "' layers.");
    for (unsigned i = 0; i < modifiers.size(); ++i)
    {
        LOG_DEBUG("Adding layer with stackable type "
                  << modifiers[i].stackableType
                  << " and effect type " << modifiers[i].effectType << ".");
        mMods.push_back(new AttributeModifiersEffect(modifiers[i].stackableType,
                                                     modifiers[i].effectType));
        LOG_DEBUG("Layer added.");
    }
    mBase = checkBounds(mBase);
}

Attribute::~Attribute()
{
//    for (std::vector<AttributeModifiersEffect *>::iterator it = mMods.begin(),
//         it_end = mMods.end(); it != it_end; ++it)
//    {
        // ?
        //delete *it;
//    }
}

bool Attribute::tick()
{
    bool ret = false;
    double prev = mBase;
    for (std::vector<AttributeModifiersEffect *>::iterator it = mMods.begin(),
        it_end = mMods.end(); it != it_end; ++it)
    {
        if ((*it)->tick())
        {
            LOG_DEBUG("Attribute layer " << mMods.begin() - it
                      << " has expiring modifiers.");
            ret = true;
        }
        if (ret)
            if (!(*it)->recalculateModifiedValue(prev)) ret = false;
        prev = (*it)->getCachedModifiedValue();
    }
    return ret;
}

void Attribute::clearMods()
{
    for (std::vector<AttributeModifiersEffect *>::iterator it = mMods.begin(),
         it_end = mMods.end(); it != it_end; ++it)
        (*it)->clearMods(mBase);
}

void Attribute::setBase(double base)
{
    base = checkBounds(base);
    LOG_DEBUG("Setting base attribute from " << mBase << " to " << base << ".");
    double prev = mBase = base;

    std::vector<AttributeModifiersEffect *>::iterator it = mMods.begin();
    while (it != mMods.end())
    {
        if ((*it)->recalculateModifiedValue(prev))
            prev = (*it++)->getCachedModifiedValue();
        else
            break;
    }
}

void AttributeModifiersEffect::clearMods(double baseValue)
{
    mStates.clear();
    mCacheVal = baseValue;
    mMod = mEffectType == Additive ? 0 : 1;
}

double Attribute::checkBounds(double baseValue) const
{
    LOG_DEBUG("Checking bounds for value: " << baseValue);
    if (baseValue > mMaxValue)
        baseValue = mMaxValue;
    else if (baseValue < mMinValue)
        baseValue = mMinValue;
    return baseValue;
}
