/*************************************************************************
 *
 * REALM CONFIDENTIAL
 * __________________
 *
 *  [2011] - [2012] Realm Inc
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Realm Incorporated and its suppliers,
 * if any.  The intellectual and technical concepts contained
 * herein are proprietary to Realm Incorporated
 * and its suppliers and may be covered by U.S. and Foreign Patents,
 * patents in process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Realm Incorporated.
 *
 **************************************************************************/
#ifndef REALM_LANG_BIND_HELPER_HPP
#define REALM_LANG_BIND_HELPER_HPP

#include <cstddef>

#include <realm/table.hpp>
#include <realm/table_view.hpp>
#include <realm/link_view.hpp>
#include <realm/group.hpp>
#include <realm/group_shared.hpp>

#include <realm/replication.hpp>

namespace realm {


/// These functions are only to be used by language bindings to gain
/// access to certain memebers that are othewise private.
///
/// \note Applications are not supposed to call any of these functions
/// directly.
///
/// All of the get_subtable_ptr() functions bind the table accessor pointer
/// before it is returned (bind_table_ptr()). The caller is then responsible for
/// making the corresponding call to unbind_table_ptr().
class LangBindHelper {
public:
    /// Increment the reference counter of the specified table accessor. This is
    /// done automatically by all of the functions in this class that return
    /// table accessor pointers, but if the binding/application makes a copy of
    /// such a pointer, and the copy needs to have an "independent life", then
    /// the binding/application must bind that copy using this function.
    static void bind_table_ptr(const Table*) noexcept;

    /// Decrement the reference counter of the specified table accessor. The
    /// binding/application must call this function for every bound table
    /// accessor pointer object, when that pointer object ends its life.
    static void unbind_table_ptr(const Table*) noexcept;

    /// Construct a new freestanding table. The table accessor pointer is bound
    /// by the callee before it is returned (bind_table_ptr()).
    static Table* new_table();

    /// Construct a new freestanding table as a copy of the specified one. The
    /// table accessor pointer is bound by the callee before it is returned
    /// (bind_table_ptr()).
    static Table* copy_table(const Table&);

    //@{

    /// These functions are like their namesakes in Group, but these bypass the
    /// construction of a smart-pointer object (TableRef). The table accessor
    /// pointer is bound by the callee before it is returned (bind_table_ptr()).

    static Table* get_table(Group&, std::size_t index_in_group);
    static const Table* get_table(const Group&, std::size_t index_in_group);

    static Table* get_table(Group&, StringData name);
    static const Table* get_table(const Group&, StringData name);

    static Table* add_table(Group&, StringData name, bool require_unique_name = true);
    static Table* get_or_add_table(Group&, StringData name, bool* was_added = 0);

    //@}

    static Table* get_subtable_ptr(Table*, std::size_t column_ndx, std::size_t row_ndx);
    static const Table* get_subtable_ptr(const Table*, std::size_t column_ndx,
                                         std::size_t row_ndx);

    // FIXME: This is an 'oddball', do we really need it? If we do,
    // please provide a comment that explains why it is needed!
    static Table* get_subtable_ptr_during_insert(Table*, std::size_t col_ndx,
                                                 std::size_t row_ndx);

    static Table* get_subtable_ptr(TableView*, std::size_t column_ndx, std::size_t row_ndx);
    static const Table* get_subtable_ptr(const TableView*, std::size_t column_ndx,
                                         std::size_t row_ndx);
    static const Table* get_subtable_ptr(const ConstTableView*, std::size_t column_ndx,
                                         std::size_t row_ndx);

    /// Calls parent.set_mixed_subtable(col_ndx, row_ndx, &source). Note
    /// that the source table must have a descriptor that is
    /// compatible with the target subtable column.
    static void set_mixed_subtable(Table& parent, std::size_t col_ndx, std::size_t row_ndx,
                                   const Table& source);

    static LinkView* get_linklist_ptr(Row&, std::size_t col_ndx);
    static void unbind_linklist_ptr(LinkView*);

    //@{

    /// Continuous transactions.
    ///
    /// advance_read() is equivalent to terminating the current read transaction
    /// (SharedGroup::end_read()), and initiating a new one
    /// (SharedGroup::begin_read()), except that all subordinate accessors
    /// (Table, Row, Descriptor) will remain attached to the underlying objects,
    /// unless those objects were removed in the target snapshot. By default,
    /// the read transaction is advanced to the latest available snapshot, but
    /// see SharedGroup::begin_read() for information about \a version.
    ///
    /// promote_to_write() is equivalent to terminating the current read
    /// transaction (SharedGroup::end_read()), and initiating a new write
    /// transaction (SharedGroup::begin_write()), except that all subordinate
    /// accessors (Table, Row, Descriptor) will remain attached to the
    /// underlying objects, unless those objects were removed in the target
    /// snapshot.
    ///
    /// commit_and_continue_as_read() is equivalent to committing the current
    /// write transaction (SharedGroup::commit()) and initiating a new read
    /// transaction, which is bound to the snapshot produced by the write
    /// transaction (SharedGroup::begin_read()), except that all subordinate
    /// accessors (Table, Row, Descriptor) will remain attached to the
    /// underlying objects.
    ///
    /// rollback_and_continue_as_read() is equivalent to rolling back the
    /// current write transaction (SharedGroup::rollback()) and initiating a new
    /// read transaction, which is bound to the snapshot, that the write
    /// transaction was based on (SharedGroup::begin_read()), except that all
    /// subordinate accessors (Table, Row, Descriptor) will remain attached to
    /// the underlying objects, unless they were attached to object that were
    /// added during the rolled back transaction.
    ///
    /// If advance_read(), promote_to_write(), commit_and_continue_as_read(), or
    /// rollback_and_continue_as_read() throws, the associated group accessor
    /// and all of its subordinate accessors are left in a state that may not be
    /// fully consistent. Only minimal consistency is guaranteed (see
    /// AccessorConsistencyLevels). In this case, the application is required to
    /// either destroy the SharedGroup object, forcing all associated accessors
    /// to become detached, or take some other equivalent action that involves a
    /// complete accessor detachment, such as terminating the transaction in
    /// progress. Until then it is an error, and unsafe if the application
    /// attempts to access any of those accessors.
    ///
    /// The application must use SharedGroup::end_read() if it wants to
    /// terminate the transaction after advance_read() or promote_to_write() has
    /// thrown an exception. Likewise, it must use SharedGroup::rollback() if it
    /// wants to terminate the transaction after commit_and_continue_as_read()
    /// or rollback_and_continue_as_read() has thrown an exception.
    ///
    /// \param history The modification history accessor associated with the
    /// specified SharedGroup object.
    ///
    /// \param observer An optional custom replication instruction handler. The
    /// application may pass such a handler to observe the sequence of
    /// modifications that advances (or rolls back) the state of the Realm.
    ///
    /// \throw SharedGroup::BadVersion Thrown by advance_read() if the specified
    /// version does not correspond to a bound (or tethered) snapshot.

    static void advance_read(SharedGroup&, History&,
                             SharedGroup::VersionID version = SharedGroup::VersionID());
    template<class O>
    static void advance_read(SharedGroup&, History&, O&& observer,
                             SharedGroup::VersionID version = SharedGroup::VersionID());
    static void promote_to_write(SharedGroup&, History&);
    template<class O>
    static void promote_to_write(SharedGroup&, History&, O&& observer);
    static void commit_and_continue_as_read(SharedGroup&);
    static void rollback_and_continue_as_read(SharedGroup&, History&);
    template<class O>
    static void rollback_and_continue_as_read(SharedGroup&, History&, O&& observer);

    //@}

    static Replication::version_type get_current_version(SharedGroup& sg)
    {
        return Replication::version_type(sg.get_current_version());
    }

    /// Returns the name of the specified data type as follows:
    ///
    /// <pre>
    ///
    ///   type_Int       ->  "int"
    ///   type_Bool      ->  "bool"
    ///   type_Float     ->  "float"
    ///   type_Double    ->  "double"
    ///   type_String    ->  "string"
    ///   type_Binary    ->  "binary"
    ///   type_DateTime  ->  "date"
    ///   type_Table     ->  "table"
    ///   type_Mixed     ->  "mixed"
    ///
    /// </pre>
    static const char* get_data_type_name(DataType) noexcept;
};




// Implementation:

inline Table* LangBindHelper::new_table()
{
    typedef _impl::TableFriend tf;
    Allocator& alloc = Allocator::get_default();
    std::size_t ref = tf::create_empty_table(alloc); // Throws
    Table::Parent* parent = nullptr;
    std::size_t ndx_in_parent = 0;
    Table* table = tf::create_accessor(alloc, ref, parent, ndx_in_parent); // Throws
    tf::bind_ref(*table);
    return table;
}

inline Table* LangBindHelper::copy_table(const Table& table)
{
    typedef _impl::TableFriend tf;
    Allocator& alloc = Allocator::get_default();
    std::size_t ref = tf::clone(table, alloc); // Throws
    Table::Parent* parent = nullptr;
    std::size_t ndx_in_parent = 0;
    Table* copy_of_table = tf::create_accessor(alloc, ref, parent, ndx_in_parent); // Throws
    tf::bind_ref(*copy_of_table);
    return copy_of_table;
}

inline Table* LangBindHelper::get_subtable_ptr(Table* t, std::size_t column_ndx,
                                               std::size_t row_ndx)
{
    Table* subtab = t->get_subtable_ptr(column_ndx, row_ndx); // Throws
    subtab->bind_ref();
    return subtab;
}

inline const Table* LangBindHelper::get_subtable_ptr(const Table* t, std::size_t column_ndx,
                                                     std::size_t row_ndx)
{
    const Table* subtab = t->get_subtable_ptr(column_ndx, row_ndx); // Throws
    subtab->bind_ref();
    return subtab;
}

inline Table* LangBindHelper::get_subtable_ptr(TableView* tv, std::size_t column_ndx,
                                               std::size_t row_ndx)
{
    return get_subtable_ptr(&tv->get_parent(), column_ndx, tv->get_source_ndx(row_ndx));
}

inline const Table* LangBindHelper::get_subtable_ptr(const TableView* tv, std::size_t column_ndx,
                                                     std::size_t row_ndx)
{
    return get_subtable_ptr(&tv->get_parent(), column_ndx, tv->get_source_ndx(row_ndx));
}

inline const Table* LangBindHelper::get_subtable_ptr(const ConstTableView* tv,
                                                     std::size_t column_ndx, std::size_t row_ndx)
{
    return get_subtable_ptr(&tv->get_parent(), column_ndx, tv->get_source_ndx(row_ndx));
}

inline Table* LangBindHelper::get_table(Group& group, std::size_t index_in_group)
{
    typedef _impl::GroupFriend gf;
    Table* table = &gf::get_table(group, index_in_group); // Throws
    table->bind_ref();
    return table;
}

inline const Table* LangBindHelper::get_table(const Group& group, std::size_t index_in_group)
{
    typedef _impl::GroupFriend gf;
    const Table* table = &gf::get_table(group, index_in_group); // Throws
    table->bind_ref();
    return table;
}

inline Table* LangBindHelper::get_table(Group& group, StringData name)
{
    typedef _impl::GroupFriend gf;
    Table* table = gf::get_table(group, name); // Throws
    if (table)
        table->bind_ref();
    return table;
}

inline const Table* LangBindHelper::get_table(const Group& group, StringData name)
{
    typedef _impl::GroupFriend gf;
    const Table* table = gf::get_table(group, name); // Throws
    if (table)
        table->bind_ref();
    return table;
}

inline Table* LangBindHelper::add_table(Group& group, StringData name, bool require_unique_name)
{
    typedef _impl::GroupFriend gf;
    Table* table = &gf::add_table(group, name, require_unique_name); // Throws
    table->bind_ref();
    return table;
}

inline Table* LangBindHelper::get_or_add_table(Group& group, StringData name, bool* was_added)
{
    typedef _impl::GroupFriend gf;
    Table* table = &gf::get_or_add_table(group, name, was_added); // Throws
    table->bind_ref();
    return table;
}

inline void LangBindHelper::unbind_table_ptr(const Table* t) noexcept
{
   t->unbind_ref();
}

inline void LangBindHelper::bind_table_ptr(const Table* t) noexcept
{
   t->bind_ref();
}

inline void LangBindHelper::set_mixed_subtable(Table& parent, std::size_t col_ndx,
                                               std::size_t row_ndx, const Table& source)
{
    parent.set_mixed_subtable(col_ndx, row_ndx, &source);
}

inline LinkView* LangBindHelper::get_linklist_ptr(Row& row, std::size_t col_ndx)
{
    LinkViewRef link_view = row.get_linklist(col_ndx);
    link_view->bind_ref();
    return &*link_view;
}

inline void LangBindHelper::unbind_linklist_ptr(LinkView* link_view)
{
   link_view->unbind_ref();
}

inline void LangBindHelper::advance_read(SharedGroup& sg, History& history,
                                         SharedGroup::VersionID version)
{
    using sgf = _impl::SharedGroupFriend;
    _impl::NullInstructionObserver* observer = 0;
    sgf::advance_read(sg, history, observer, version);
}

template<class O>
inline void LangBindHelper::advance_read(SharedGroup& sg, History& history, O&& observer,
                                         SharedGroup::VersionID version)
{
    using sgf = _impl::SharedGroupFriend;
    sgf::advance_read(sg, history, &observer, version);
}

inline void LangBindHelper::promote_to_write(SharedGroup& sg, History& history)
{
    using sgf = _impl::SharedGroupFriend;
    _impl::NullInstructionObserver* observer = 0;
    sgf::promote_to_write(sg, history, observer);
}

template<class O>
inline void LangBindHelper::promote_to_write(SharedGroup& sg, History& history, O&& observer)
{
    using sgf = _impl::SharedGroupFriend;
    sgf::promote_to_write(sg, history, &observer);
}

inline void LangBindHelper::commit_and_continue_as_read(SharedGroup& sg)
{
    using sgf = _impl::SharedGroupFriend;
    sgf::commit_and_continue_as_read(sg);
}

inline void LangBindHelper::rollback_and_continue_as_read(SharedGroup& sg, History& history)
{
    using sgf = _impl::SharedGroupFriend;
    _impl::NullInstructionObserver* observer = 0;
    sgf::rollback_and_continue_as_read(sg, history, observer);
}

template<class O>
inline void LangBindHelper::rollback_and_continue_as_read(SharedGroup& sg, History& history,
                                                          O&& observer)
{
    using sgf = _impl::SharedGroupFriend;
    sgf::rollback_and_continue_as_read(sg, history, &observer);
}

} // namespace realm

#endif // REALM_LANG_BIND_HELPER_HPP
