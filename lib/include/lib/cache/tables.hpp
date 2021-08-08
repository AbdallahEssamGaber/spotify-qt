#pragma once

#include "lib/spotify/track.hpp"

#include "thirdparty/sqlite_orm.h"

namespace orm = sqlite_orm;

namespace lib
{
	namespace db
	{
		using tracks = orm::internal::table_t<lib::spt::track,
			decltype(orm::make_column("id", &lib::spt::track::id, orm::primary_key())),
			decltype(orm::make_column("name", &lib::spt::track::name)),
			decltype(orm::make_column("is_local", &lib::spt::track::is_local)),
			decltype(orm::make_column("is_playable", &lib::spt::track::is_playable)),
			decltype(orm::make_column("duration", &lib::spt::track::duration)),
			decltype(orm::make_column("added_at", &lib::spt::track::added_at)),
			decltype(orm::make_column("image", &lib::spt::track::image))>;

		using storage = orm::internal::storage_t<tracks>;
	}
}
