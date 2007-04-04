/// @file
///
/// MEDataSource: Allow access to a source of visibility data, probably
/// either a MeasurementSet or a stream.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef MEDATASOURCE_H_
#define MEDATASOURCE_H_

// boost includes
#include <boost/shared_ptr.hpp>

// CASA includes
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MRadialVelocity.h>

// own includes
#include "MEDataIterator.h"
#include "IDataSelector.h"
#include "IDataConverter.h"

//#include "METableDataAccessor.h"

namespace conrad {
class MEDataSource
{
public:
	
	/// an empty virtual destructor to make the compiler happy
	virtual ~MEDataSource();

	/// create a converter object corresponding to this type of the
	/// DataSource. The user can change converting policies (units,
	/// reference frames) by appropriate calls to this converter object
	/// and pass it back to createIterator(...). The data returned by
	/// the iteratsr will automatically be in the requested frame/units
	///
	/// @return a shared pointer to a new DataConverter object
	///
	/// The method acts as a factory by creating a new DataConverter.
	/// The lifetime of this converter is the same as the lifetime of the
	/// DataSource object. Therefore, it can be reused multiple times,
	/// if necessary. However, the behavior of iterators created
	/// with a particular DataConverter is undefined, if you change
	/// the DataConverter after the creation of an iterator, unless you
	/// call init() of the iterator (and start a new iteration loop).
	virtual boost::shared_ptr<IDataConverter> createConverter() const = 0;

	/// get iterator over the whole dataset represented by this DataSource
	/// object. Default data conversion policies will be used, see
	/// IDataConverter.h for default values. Default implementation is
	/// via the most general createIterator(...) call, override it in 
	/// derived classes, if a (bit) higher performance is required
	///
	/// @return a shared pointer to DataIterator object
	///
	/// The method acts as a factory by creating a new DataIterator.
	/// The lifetime of this iterator is the same as the lifetime of
	/// the DataSource object. Therefore, it can be reused multiple times,
	/// if necessary. 
	virtual boost::shared_ptr<MEDataIterator> createIterator() const;

	/// get iterator over the whole dataset with explicitly specified
	/// conversion policies. Default implementation is via the most 
	/// general createIterator(...) call, override it in the derived
	/// classes, if a (bit) higer performance is required
	///
	/// @param[in] conv a shared pointer to the converter object defining
	///            reference frames and units to be used
	/// @return a shared pointer to DataIterator object
	///
	/// The method acts as a factory by creating a new DataIterator.
	/// The lifetime of this iterator is the same as the lifetime of
	/// the DataSource object. Therefore, it can be reused multiple times,
	/// if necessary. 
	virtual boost::shared_ptr<MEDataIterator> createIterator(const
                    boost::shared_ptr<IDataConverter const> &conv) const;
	
	
	/// this variant of createIterator is defined to force the type
	/// conversion between the non-const and const smart pointers 
	/// explicitly. Otherwise, method overloading doesn't work because
	/// the compiler tries to build a template for interconversion
	/// between IDataConverter and IDataSelector
	/// 
	/// This version just calls the appropriate virtual function and
	/// shouldn't add any overheads, provided the compiler can optimize
	/// inline methods properly
	inline boost::shared_ptr<MEDataIterator> createIterator(const
		    boost::shared_ptr<IDataConverter> &conv) const { 
            return createIterator(static_cast<const 
	            boost::shared_ptr<IDataConverter const>&>(conv)); 
        }

	/// get iterator over a selected part of the dataset represented
	/// by this DataSource object. Default data conversion policies
	/// will be used, see IDataConverter.h for default values.
	/// The default implementation is via the most general 
	/// createIterator(...) call, override it in derived classes, 
	/// if a (bit) higher performance is required
	///
	/// @param[in] sel a shared pointer to the selector object defining 
	///            which subset of the data is used
	/// @return a shared pointer to DataIterator object
	///
	/// The method acts as a factory by creating a new DataIterator.
	/// The lifetime of this iterator is the same as the lifetime of
	/// the DataSource object. Therefore, it can be reused multiple times,
	/// if necessary. 
	virtual boost::shared_ptr<MEDataIterator> createIterator(const
	           boost::shared_ptr<IDataSelector const> &sel) const;

	/// this variant of createIterator is defined to force the type
	/// conversion between the non-const and const smart pointers 
	/// explicitly. Otherwise, method overloading doesn't work because
	/// the compiler tries to build a template for interconversion
	/// between IDataConverter and IDataSelector
	/// 
	/// This version just calls the appropriate virtual function and
	/// shouldn't add any overheads, provided the compiler can optimize
	/// inline methods properly
	inline boost::shared_ptr<MEDataIterator> createIterator(const
		    boost::shared_ptr<IDataSelector> &sel) const { 
            return createIterator(static_cast<const 
                    boost::shared_ptr<IDataSelector const>&>(sel)); 
        }

	/// get iterator over a selected part of the dataset represented
	/// by this DataSource object with an explicitly specified conversion
	/// policy. This is the most general createIterator(...) call, which
	/// is used as a default implementation for all less general cases
	/// (although they can be overriden in the derived classes, if it 
	//  will be necessary because of the performance issues)
	///
	/// @param[in] sel a shared pointer to the selector object defining 
	///            which subset of the data is used
	/// @param[in] conv a shared pointer to the converter object defining
	///            reference frames and units to be used
	/// @return a shared pointer to DataIterator object
	///
	/// The method acts as a factory by creating a new DataIterator.
	/// The lifetime of this iterator is the same as the lifetime of
	/// the DataSource object. Therefore, it can be reused multiple times,
	/// if necessary. Call init() to rewind the iterator.
	virtual boost::shared_ptr<MEDataIterator> createIterator(const
	           boost::shared_ptr<IDataSelector const> &sel, const
		   boost::shared_ptr<IDataConverter const> &conv) const = 0;

	/// create a selector object corresponding to this type of the
	/// DataSource
	///
	/// @return a shared pointer to the DataSelector corresponding to
	/// this type of DataSource. DataSource acts as a factory and
	/// creates a selector object of the appropriate type
	///
	/// This method acts as a factory by creating a new DataSelector
	/// appropriate to the given DataSource. The lifetime of the
	/// DataSelector is the same as the lifetime of the DataSource 
	/// object. Therefore, it can be reused multiple times,
	/// if necessary. However, the behavior of iterators already obtained
	/// with this DataSelector is undefined, if one changes the selection
	/// unless the init method is called for the iterator (and the new
	/// iteration loop is started).
	virtual boost::shared_ptr<IDataSelector> createSelector() const = 0;
};
}
#endif /*MEDATASOURCE_H_*/
