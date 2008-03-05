/// @file IDataIterator.h
/// @brief an interface to the data iterator with associated buffers.
/// @details See the description of IConstDataIterator
/// for more details. Buffers are the visibility chunks conformant
/// to the data pointed to by the IDataIterator, but with a read/write access. 
/// They are managed by the DataSource object and will not be destroyed
/// when the iterator goes out of scope. All iterators created from the same
/// DataSource object work with the same buffers. The user is responsible
/// for synchronization, if a simultanous access to the same buffer is
/// implemented in a parallel environment. The user should also take care
/// in the situation when the iterators with different selection access
/// the same buffer (this behavior is still TBD).
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_DATA_ITERATOR_H
#define I_DATA_ITERATOR_H

#include <string>

#include <dataaccess/IConstDataIterator.h>
#include <dataaccess/IDataAccessor.h>

namespace askap {

namespace synthesis {

/// @brief an interface to the data iterator with associated buffers.
/// @details See the description of IConstDataIterator
/// for more details. Buffers are the visibility chunks conformant
/// to the data pointed to by the IDataIterator, but with a read/write access. 
/// They are managed by the DataSource object and will not be destroyed
/// when the iterator goes out of scope. All iterators created from the same
/// DataSource object work with the same buffers. The user is responsible
/// for synchronization, if a simultanous access to the same buffer is
/// implemented in a parallel environment. The user should also take care
/// in the situation when the iterators with different selection access
/// the same buffer (this behavior is still TBD).
/// @ingroup dataaccess_i
class IDataIterator : virtual public IConstDataIterator
{
public:
	/// the type of the value pointed by this iterator
	typedef IDataAccessor& value_type;
	
        /// the type of the pointer returned by operator->
        /// We can't generally just use value_type * because
        typedef IDataAccessor* pointer_type;
	
	/// Return the data accessor (current chunk) in various ways
	
	/// operator* delivers a reference to data accessor (current chunk)
	///
	/// @return a reference to the current chunk
	///
	/// constness of the return type is changed to allow read/write
	/// operations.
	///
	virtual IDataAccessor& operator*() const = 0;
	
	/// operator-> delivers a pointer. 
	///
	/// @return a pointer to the current chunk
	///
	/// Allows the syntax like it->visibility()
	/// The default implementation works via operator*, however to 
	/// avoid an additional function call, the method
	/// can be specialized in the derived classes
	///
	/// constness of the return type is changed to allow read/write
	/// operations.
	///
	virtual IDataAccessor* operator->() const;

	/// Switch the output of operator* and operator-> to one of 
	/// the buffers. This is meant to be done to provide the same 
	/// interface for a buffer access as exists for the original 
	/// visibilities (e.g. it->visibility() to get the cube).
	/// It can be used for an easy substitution of the original 
	/// visibilities to ones stored in a buffer, when the iterator is
	/// passed as a parameter to mathematical algorithms. 
	/// 
	/// The operator* and operator-> will refer to the chosen buffer
	/// until a new buffer is selected or the chooseOriginal() method
	/// is executed to revert operators to their default meaning
	/// (to refer to the primary visibility data).
	///
	/// @param[in] bufferID  the name of the buffer to choose
	///
	virtual void chooseBuffer(const std::string &bufferID) = 0;

	/// Switch the output of operator* and operator-> to the original
	/// state (present after the iterator is just constructed) 
	/// where they point to the primary visibility data. This method
	/// is indended to cancel the results of chooseBuffer(casa::uInt)
	///
	virtual void chooseOriginal() = 0;

	/// return any associated buffer for read/write access. The 
	/// buffer is identified by its bufferID. The method 
	/// ignores a chooseBuffer/chooseOriginal setting.
	/// 
	/// @param[in] bufferID the name of the buffer requested
	/// @return a reference to writable data accessor to the
	///         buffer requested
	///
	/// Because IDataAccessor has both const and non-const visibility()
	/// methods defined separately, it is possible to detect when a
	/// write operation took place and implement a delayed writing
	virtual IDataAccessor& buffer(const std::string &bufferID) const = 0;

	/// advance the iterator one step further
	///
	/// @return A reference to itself (to allow ++++it synthax)
	///
	/// The default implementation is via next(), however one can
	/// override this method in a derived class to avoid this (slight)
	/// overhead. This method overrides the the method of the base
	/// class to return the correct type 
	virtual IDataIterator& operator++();
};

} // end of namespace synthesis

} // end of namespace askap
#endif // #ifndef I_DATA_ITERATOR_H
