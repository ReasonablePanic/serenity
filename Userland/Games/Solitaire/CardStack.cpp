/*
 * Copyright (c) 2020, Till Mayer <till.mayer@web.de>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "CardStack.h"

namespace Solitaire {

CardStack::CardStack()
    : m_position({ 0, 0 })
    , m_type(Invalid)
    , m_base(m_position, { Card::width, Card::height })
{
}

CardStack::CardStack(const Gfx::IntPoint& position, Type type)
    : m_position(position)
    , m_type(type)
    , m_rules(rules_for_type(type))
    , m_base(m_position, { Card::width, Card::height })
{
    VERIFY(type != Invalid);
    calculate_bounding_box();
}

void CardStack::clear()
{
    m_stack.clear();
    m_stack_positions.clear();
}

void CardStack::draw(GUI::Painter& painter, const Gfx::Color& background_color)
{
    switch (m_type) {
    case Stock:
        if (is_empty()) {
            painter.fill_rect(m_base.shrunken(Card::width / 4, Card::height / 4), background_color.lightened(1.5));
            painter.fill_rect(m_base.shrunken(Card::width / 2, Card::height / 2), background_color);
            painter.draw_rect(m_base, background_color.darkened(0.5));
        }
        break;
    case Foundation:
        if (is_empty() || (m_stack.size() == 1 && peek().is_moving())) {
            painter.draw_rect(m_base, background_color.darkened(0.5));
            for (int y = 0; y < (m_base.height() - 4) / 8; ++y) {
                for (int x = 0; x < (m_base.width() - 4) / 5; ++x) {
                    painter.draw_rect({ 4 + m_base.x() + x * 5, 4 + m_base.y() + y * 8, 1, 1 }, background_color.darkened(0.5));
                }
            }
        }
        break;
    case Waste:
        break;
    case Play:
        if (is_empty() || (m_stack.size() == 1 && peek().is_moving()))
            painter.draw_rect(m_base, background_color.darkened(0.5));
        break;
    case Normal:
        painter.draw_rect(m_base, background_color.darkened(0.5));
        break;
    default:
        VERIFY_NOT_REACHED();
    }

    if (is_empty())
        return;

    if (m_rules.shift_x == 0 && m_rules.shift_y == 0) {
        auto& card = peek();
        card.draw(painter);
        return;
    }

    for (auto& card : m_stack) {
        if (!card.is_moving())
            card.clear_and_draw(painter, background_color);
    }
}

void CardStack::rebound_cards()
{
    VERIFY(m_stack_positions.size() == m_stack.size());

    size_t card_index = 0;
    for (auto& card : m_stack)
        card.set_position(m_stack_positions.at(card_index++));
}

void CardStack::add_all_grabbed_cards(const Gfx::IntPoint& click_location, NonnullRefPtrVector<Card>& grabbed)
{
    VERIFY(grabbed.is_empty());

    if (m_type != Normal) {
        auto& top_card = peek();
        if (top_card.rect().contains(click_location)) {
            top_card.set_moving(true);
            grabbed.append(top_card);
        }
        return;
    }

    RefPtr<Card> last_intersect;

    for (auto& card : m_stack) {
        if (card.rect().contains(click_location)) {
            if (card.is_upside_down())
                continue;

            last_intersect = card;
        } else if (!last_intersect.is_null()) {
            if (grabbed.is_empty()) {
                grabbed.append(*last_intersect);
                last_intersect->set_moving(true);
            }

            if (card.is_upside_down()) {
                grabbed.clear();
                return;
            }

            card.set_moving(true);
            grabbed.append(card);
        }
    }

    if (grabbed.is_empty() && !last_intersect.is_null()) {
        grabbed.append(*last_intersect);
        last_intersect->set_moving(true);
    }
}

bool CardStack::is_allowed_to_push(const Card& card, size_t stack_size) const
{
    if (m_type == Stock || m_type == Waste || m_type == Play)
        return false;

    if (m_type == Normal && is_empty())
        return card.value() == 12;

    if (m_type == Foundation && is_empty())
        return card.value() == 0;

    if (!is_empty()) {
        auto& top_card = peek();
        if (top_card.is_upside_down())
            return false;

        if (m_type == Foundation) {
            // Prevent player from dragging an entire stack of cards to the foundation stack
            if (stack_size > 1)
                return false;
            return top_card.type() == card.type() && m_stack.size() == card.value();
        } else if (m_type == Normal) {
            return top_card.color() != card.color() && top_card.value() == card.value() + 1;
        }

        VERIFY_NOT_REACHED();
    }

    return true;
}

void CardStack::push(NonnullRefPtr<Card> card)
{
    auto size = m_stack.size();
    auto top_most_position = m_stack_positions.is_empty() ? m_position : m_stack_positions.last();

    if (size && size % m_rules.step == 0) {
        if (peek().is_upside_down())
            top_most_position.translate_by(m_rules.shift_x, m_rules.shift_y_upside_down);
        else
            top_most_position.translate_by(m_rules.shift_x, m_rules.shift_y);
    }

    if (m_type == Stock)
        card->set_upside_down(true);

    card->set_position(top_most_position);

    m_stack.append(card);
    m_stack_positions.append(top_most_position);
    calculate_bounding_box();
}

NonnullRefPtr<Card> CardStack::pop()
{
    auto card = m_stack.take_last();

    calculate_bounding_box();
    if (m_type == Stock)
        card->set_upside_down(false);

    m_stack_positions.take_last();
    return card;
}

void CardStack::move_to_stack(CardStack& stack)
{
    while (!m_stack.is_empty()) {
        auto card = m_stack.take_first();
        m_stack_positions.take_first();
        stack.push(move(card));
    }

    calculate_bounding_box();
}

void CardStack::calculate_bounding_box()
{
    m_bounding_box = Gfx::IntRect(m_position, { Card::width, Card::height });

    if (m_stack.is_empty())
        return;

    uint16_t width = 0;
    uint16_t height = 0;
    size_t card_position = 0;
    for (auto& card : m_stack) {
        if (card_position % m_rules.step == 0 && card_position) {
            if (card.is_upside_down()) {
                width += m_rules.shift_x;
                height += m_rules.shift_y_upside_down;
            } else {
                width += m_rules.shift_x;
                height += m_rules.shift_y;
            }
        }
        ++card_position;
    }

    m_bounding_box.set_size(Card::width + width, Card::height + height);
}

}
