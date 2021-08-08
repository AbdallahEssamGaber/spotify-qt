#pragma once

#include "thirdparty/sqlite_orm.h"
#include "lib/enum/albumgroup.hpp"

namespace sqlite_orm
{
	//region lib::album_group

	template<>
	struct type_printer<lib::album_group>: public integer_printer
	{
	};

	template<>
	struct statement_binder<lib::album_group>
	{
		auto bind(sqlite3_stmt *stmt, int index, const lib::album_group &value) -> int
		{
			return statement_binder<int>().bind(stmt, index, static_cast<int>(value));
		}
	};

	template<>
	struct field_printer<lib::album_group>
	{
		auto operator()(const lib::album_group &value) const -> int
		{
			return static_cast<int>(value);
		}
	};

	template<>
	struct row_extractor<lib::album_group>
	{
		auto extract(int row_value) -> lib::album_group
		{
			return static_cast<lib::album_group>(row_value);
		}

		auto extract(sqlite3_stmt *stmt, int column_index) -> lib::album_group
		{
			return extract(sqlite3_column_int(stmt, column_index));
		}
	};

	//endregion
}
