// Copyright 2023 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>

#pragma once
#include "warning-disable.hpp"
#include <atomic>
#include <cinttypes>
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include "warning-enable.hpp"

namespace tonplugins::memory {

	template<typename T>
	class ring {
		typedef std::function<void(tonplugins::memory::ring<T>&)> ring_listener_t;

		T*     _buffer;
		size_t _size;

		std::atomic_size_t _write_pos;
		std::atomic_size_t _read_pos;

		std::shared_ptr<void> _internal_data;

		std::map<size_t, ring_listener_t> _notifications;
		size_t                            _notification_id;

		public:
		ring(size_t elements);
		~ring();

		/** Write data into the ring buffer.
		 *
		 * If the read pointer is inside the region to be written, it will be advanced to be just outside of that region.
		 *
		 * @argument size The size (in elements) of the buffer to be written, limited by the ring buffer size.
		 * @argument buffer The buffer to copy data from. Must be at least the size specified in length.
		 * @return The number of elements written into the ring buffer.
		 */
		size_t write(size_t size, T const* buffer);

		size_t write(std::vector<T> const& buffer, size_t offset = 0)
		{
			return write(buffer.size(), &(buffer.at(offset)));
		}

		/** Peek at data in the ring buffer.
		 *
		 * Confirm the peek with read(size, nullptr).
		 * 
		 * \param[in] size The minimum length of data that should be available.
		 * \return `nullptr` if the length constraint can't be fulfilled, otherwise a pointer.
         */
		T const* peek(size_t size);

		/** Poke data into the ring buffer.
		 *
		 * Confirm the poke with write(size, nullptr).
		 *
		 * \param[in] size The length of data you wish to poke.
		 * \return A pointer to the data.
		 */
		T* poke(size_t size);

		/** Read data from the ring buffer.
		 *
		 * @argument size The size (in elements) of the buffer to be read to, limited by the ring buffer size.
		 * @argument buffer The buffer to copy data into. Must be at least the size specified in length.
		 * @return The number of bytes read.
		 */
		size_t read(size_t size, T* buffer);

		size_t read(std::vector<T>& buffer, size_t offset = 0)
		{
			return read(buffer.size(), &(buffer.at(offset)));
		}

		/** Free space of the ring buffer in number of elements.
		 *
		 * @return Number of elements currently considered free.
		 */
		size_t free()
		{
			return size() - used();
		}

		/** Used space of the ring buffer in number of elements.
		 *
		 * @return Number of elements currently considered used.
		 */
		size_t used();

		/** Total size of the ring buffer in number of elements.
		 *
		 * @return Number of elements this ring buffer can hold.
		 */
		size_t size();

		/** Listen/Silence notifications for data availability.
		 *
		 * The signal must be manually set back to false.
		 */
		size_t listen(ring_listener_t fn);
		void   silence(size_t id);
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
