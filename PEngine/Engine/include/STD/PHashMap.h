#pragma once
#include "PArray.h"
#include "Logging.h"

namespace pstd {
	template<typename T>
	size_t hash(T val);

	template<typename K, typename T>
	struct HashMap {
		struct Slot {
			K key;
			T value;
		};

		enum ControlByte : uint8_t {
			CB_EMPTY = 0xFF,
			CB_DELETED = 0xFE,
			CB_SENTINEL = 0xFD
		};

		const T& operator[](K key) const {
			size_t hash{ pstd::hash(key) };
			uint8_t byteHash{ ncast<uint8_t>(hash & 0b01111111) };

			for (size_t i{}; i < capacity; i++) {
				size_t slot{ (hash + i) % capacity };
				uint8_t slotHash{ pControls[slot] };

				if (pControls[slot] == byteHash) {
					if (pSlots[slot].key == key) {
						return pSlots[slot].value;
					}
				}
			}

			ASSERT(false, "key does not exist in map");
			return pSlots[0].value;
		}

		T& operator[](K key) {
			ASSERT(count <= capacity);

			size_t hash{ pstd::hash(key) };
			uint8_t byteHash{ ncast<uint8_t>(hash & 0b01111111) };

			for (size_t i{}; i < capacity; i++) {
				size_t slot{ (hash + i) % capacity };

				uint8_t control{ pControls[slot] };
				if (control == CB_EMPTY) {
					ASSERT(count < capacity);

					count++;
					pControls[slot] = byteHash;
					pSlots[slot] = { .key = key };
					return pSlots[slot].value;
				}

				if (control == byteHash) {
					if (pSlots[slot].key == key) {
						return pSlots[slot].value;
					}
				}
			}

			ASSERT(false, "hashmap full, cant add new item");
			return pSlots[0].value;
		}

		uint8_t* pControls;
		Slot* pSlots;
		size_t count;
		size_t capacity;
	};

	template<typename K, typename T>
	HashMap<K, T> createHashMap(pstd::Arena* pArena, size_t slotCount) {
		using Slot = typename HashMap<K, T>::Slot;
		using ControlByte = typename HashMap<K, T>::ControlByte;

		auto* pSlots{ pstd::alloc<Slot>(pArena, slotCount) };
		auto* pControlBytes{ pstd::alloc<uint8_t>(pArena, slotCount) };

		for (size_t i{}; i < slotCount; i++) {
			pControlBytes[i] = ControlByte::CB_EMPTY;
		}

		return HashMap<K, T>{ .pControls = pControlBytes,
							  .pSlots = pSlots,
							  .count = 0,
							  .capacity = slotCount };
	}

}  // namespace pstd
