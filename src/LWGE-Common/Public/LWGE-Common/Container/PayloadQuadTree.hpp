#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <concepts>
#include <span>

namespace lwge
{
    [[nodiscard]] constexpr auto get_quad_tree_element_count_by_level(uint64_t levels) noexcept
    {
        auto result = 0;
        for (auto i = 0; i < levels + 1; i++)
        {
            result <<= 2;
            result |= 0b1;
        }
        return result;
    }

    template<typename Payload, uint8_t Levels, std::unsigned_integral IndexType>
    struct FlatPayloadQuadTree
    {
        static_assert(
            std::is_same_v<IndexType, uint8_t> && (Levels <= 3) ||
            std::is_same_v<IndexType, uint16_t> && (Levels <= 7) ||
            std::is_same_v<IndexType, uint32_t> && (Levels <= 15),
            "SubDir level high enough to enable indices going out of bounds.");

        constexpr static auto START_IDX_MASK = 0x55555555u;

        enum class SubDir : IndexType
        {
            TopLeft = 0,
            TopRight = 1,
            BottomLeft = 2,
            BottomRight = 3
        };

        struct Position
        {
            uint32_t x;
            uint32_t y;
            uint32_t level;
        };

        [[nodiscard]] constexpr inline static auto
            prior_level_start_index(auto current_level_start_index) noexcept
        {
            assert((current_level_start_index & START_IDX_MASK) == current_level_start_index);
            return current_level_start_index >> 2;
        }

        [[nodiscard]] constexpr inline static auto
            next_level_start_index(auto current_level_start_index) noexcept
        {
            assert((current_level_start_index & START_IDX_MASK) == current_level_start_index);
            return (current_level_start_index << 2) | 0b1;
        }

        [[nodiscard]] constexpr inline static auto
            get_subdir_start_index(auto current_level_start_index, SubDir subdir) noexcept
        {
            assert(current_level_start_index > 0);
            assert((current_level_start_index & START_IDX_MASK) == current_level_start_index);
            auto level_index = get_level_index_from_level_start_index(current_level_start_index);
            auto level_subdir_size = get_size_of_level(level_index - 1);
            return static_cast<IndexType>(subdir) * level_subdir_size + current_level_start_index;
        }

        [[nodiscard]] constexpr inline static auto
            get_level_index_from_level_start_index(auto current_level_start_index) noexcept
        {
            assert((current_level_start_index & START_IDX_MASK) == current_level_start_index);
            return std::popcount(current_level_start_index);
        }

        [[nodiscard]] constexpr inline static auto
            get_level_start_index_from_element_index(auto index) noexcept
        {
            assert(index < get_quad_tree_element_count_by_level(Levels));
            auto clz = std::countl_zero(index);
            auto masked = (~0u >> clz);
            auto start_idx = masked & START_IDX_MASK;
            bool start_idx_larger = start_idx > index;
            return start_idx >> (start_idx_larger * 2);
        }

        [[nodiscard]] constexpr inline static auto
            get_level_relative_index(auto index) noexcept
        {
            assert(index < get_quad_tree_element_count_by_level(Levels));
            auto start_index = get_level_start_index_from_element_index(index);
            return index - start_index;
        }

        [[nodiscard]] constexpr inline static auto
            get_size_of_level(auto level_idx)
        {
            auto size = 1;
            return size << (2 * level_idx);
        }

        [[nodiscard]] constexpr inline static auto
            get_level_relative_direction(auto relative_index, auto level_idx) noexcept
        {
            assert(relative_index < get_size_of_level(level_idx));
            auto div = 1 << (2 * (level_idx - 1));
            return static_cast<SubDir>(relative_index / div);
        }

        [[nodiscard]] constexpr inline static auto
            traverse_forward(auto current_index, SubDir subdiv) noexcept
        {
            assert(current_index < get_quad_tree_element_count_by_level(Levels));

            if (current_index == 0)
            {
                return static_cast<IndexType>(subdiv) + 1;
            }

            auto current_level_start_index = get_level_start_index_from_element_index(current_index);
            auto level_index = get_level_index_from_level_start_index(current_level_start_index);
            auto level_subdir_size = get_size_of_level(level_index);

            auto start_relative_index = get_level_relative_index(current_index);
            auto start_relative_direction = get_level_relative_direction(start_relative_index, level_index);

            auto level_dir_offset = IndexType(start_relative_direction) * level_subdir_size;

            auto subdir_start_index = get_subdir_start_index(current_level_start_index, start_relative_direction);
            auto subdir_node_relative_index = current_index - subdir_start_index;
            auto subdir_node_relative_direction = static_cast<SubDir>(subdir_node_relative_index / level_subdir_size);
            auto offset = level_dir_offset + subdir_node_relative_index * 4;

            auto next_level_start_index_val = next_level_start_index(current_level_start_index);
            auto next_level_subdir_start_index = get_subdir_start_index(next_level_start_index_val, subdir_node_relative_direction);
            return next_level_subdir_start_index + static_cast<IndexType>(subdiv) + offset;
        }

        [[nodiscard]] constexpr inline static auto
            get_position_from_index(auto index) noexcept
        {
            Position result = {};

            auto current_level_start_index = get_level_start_index_from_element_index(current_index);
            auto level_index = get_level_index_from_level_start_index(current_level_start_index);
            auto level_subdir_size = get_size_of_level(level_index);

            auto start_relative_index = get_level_relative_index(index);

            Position result = {
                .x = 0,
                .y = 0,
                .level = level_index
            };
            return result;
        }

        std::array<Payload, get_quad_tree_element_count_by_level(Levels)> payloads;
    };
}
