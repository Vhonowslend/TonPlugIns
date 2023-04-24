// Copyright 2023 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>

#pragma once
#include "warning-disable.hpp"
#include <cinttypes>
#include <memory>
#include <vector>
#include "warning-enable.hpp"

namespace tonplugins::memory {
	template<typename T>
	class ring {
		size_t _size;
		size_t _write_pos;
		size_t _read_pos;

		T* _buffer;

		std::shared_ptr<void> _internal_data;

		public:
		ring(size_t minimum_size);
		~ring();

		/** Read up to 'length' elements from the ring buffer.
		 * 
		 */
		size_t read(size_t length, T* buffer);

		/** Read up to 'buffer.size()' elements from the ring buffer.
		 * 
		 */
		size_t read(std::vector<T>& buffer);

		/** Write up to 'length' elements into the ring buffer.
		 * 
		 */
		size_t write(size_t length, T const* buffer);

		/** Write up to 'buffer.size()' elements into the ring buffer.
		 * 
		 */
		size_t write(std::vector<T> const& buffer);

		/** Free space of the ring buffer in number of elements.
		 * 
		 */
		size_t free();

		/** Used space of the ring buffer in number of elements.
		 * 
		 */
		size_t used();

		/** Total size of the ring buffer in number of elements.
		 * 
		 */
		size_t size();
	};

	typedef ring<float>    float_ring_t;
	typedef ring<double>   double_ring_t;
	typedef ring<int8_t>   int8_ring_t;
	typedef ring<uint8_t>  uint8_ring_t;
	typedef ring<int16_t>  int16_ring_t;
	typedef ring<uint16_t> uint16_ring_t;
	typedef ring<int32_t>  int32_ring_t;
	typedef ring<uint32_t> uint32_ring_t;
	typedef ring<int64_t>  int64_ring_t;
	typedef ring<uint64_t> uint64_ring_t;
} // namespace tonplugins::memory
