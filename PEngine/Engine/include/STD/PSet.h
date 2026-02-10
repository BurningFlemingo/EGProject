#pragma once

#include "PArray.h"
#include "Logging.h"
#include "POptional.h"

namespace pstd {
	template<typename K>
	struct Set {
		struct Slot {
			K key;
		};

		enum ControlByte : uint8_t {
			CB_EMPTY = 0xFF,
			CB_DELETED = 0xFE,
			CB_SENTINEL = 0xFD
		};

		uint8_t* pControls;
		Slot* pSlots;
		size_t count;
		size_t capacity;
	};

	template<typename T>
	void insert(Set<T>* pSet, T key) {
		ASSERT(pSet->count <= pSet->capacity);

		size_t hash{ pstd::hash(key) };
		uint8_t byteHash{ ncast<uint8_t>(hash & 0b01111111) };

		for (size_t i{}; i < pSet->capacity; i++) {
			size_t slot{ (hash + i) % pSet->capacity };

			uint8_t control{ pSet->pControls[slot] };
			if (control == Set<T>::CB_EMPTY) {
				ASSERT(pSet->count < pSet->capacity);

				pSet->count++;
				pSet->pControls[slot] = byteHash;
				pSet->pSlots[slot] = { .key = key };

				return;
			}

			if (control == byteHash) {
				if (pSet->pSlots[slot].key == key) {
					return;
				}
			}
		}

		ASSERT(false, "set full, cant add new item");
	}

	template<typename T>
	Set<T> createSet(pstd::Arena* pArena, size_t slotCount) {
		using Slot = typename Set<T>::Slot;
		using ControlByte = typename Set<T>::ControlByte;

		auto* pSlots{ pstd::alloc<Slot>(pArena, slotCount) };
		auto* pControlBytes{ pstd::alloc<uint8_t>(pArena, slotCount) };

		for (size_t i{}; i < slotCount; i++) {
			pControlBytes[i] = ControlByte::CB_EMPTY;
		}

		return Set<T>{ .pControls = pControlBytes,
					   .pSlots = pSlots,
					   .count = 0,
					   .capacity = slotCount };
	}

	template<typename T>
	void remove(Set<T>* pSet, T key) {
		using ControlByte = typename Set<T>::ControlByte;

		size_t hash{ pstd::hash(key) };
		uint8_t byteHash{ ncast<uint8_t>(hash & 0b01111111) };

		for (size_t i{}; i < pSet->capacity; i++) {
			size_t slot{ (hash + i) % pSet->capacity };
			uint8_t slotHash{ pSet->pControls[slot] };

			if (slotHash == ControlByte::CB_EMPTY) {
				break;
			}

			if (slotHash == byteHash) {
				if (pSet->pSlots[slot].key == key) {
					ASSERT(pSet->count > 0, "set count got corrupted");

					pSet->pControls[slot] = ControlByte::CB_SENTINEL;
					pSet->count--;
				}
			}
		}
	}

	template<typename T>
	bool contains(const Set<T>& set, T key) {
		using ControlByte = typename Set<T>::ControlByte;

		size_t hash{ pstd::hash(key) };
		uint8_t byteHash{ ncast<uint8_t>(hash & 0b01111111) };

		for (size_t i{}; i < set.capacity; i++) {
			size_t slot{ (hash + i) % set.capacity };
			uint8_t slotHash{ set.pControls[slot] };

			if (slotHash == ControlByte::CB_EMPTY) {
				break;
			}

			if (slotHash == byteHash) {
				if (set.pSlots[slot].key == key) {
					return true;
				}
			}
		}

		return false;
	}

}  // namespace pstd
