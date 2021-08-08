#pragma once

#include "lib/spotify/album.hpp"
#include "lib/spotify/track.hpp"
#include "lib/spotify/artist.hpp"

#include "lib/db/image.hpp"

#include "thirdparty/sqlite_orm.h"

namespace orm = sqlite_orm;

namespace lib
{
	namespace db
	{
		using albums = orm::internal::table_t<lib::spt::album,
			decltype(orm::make_column("id", &lib::spt::album::id, orm::primary_key())),
			decltype(orm::make_column("name", &lib::spt::album::name)),
			decltype(orm::make_column("album_group", &lib::spt::album::album_group)),
			decltype(orm::make_column("image", &lib::spt::album::image)),
			decltype(orm::make_column("artist", &lib::spt::album::artist)),
			decltype(orm::make_column("release_date", &lib::spt::album::release_date))>;

		using artists = orm::internal::table_t<lib::spt::artist,
			decltype(orm::make_column("id", &lib::spt::artist::id, orm::primary_key())),
			decltype(orm::make_column("name", &lib::spt::artist::name)),
			decltype(orm::make_column("followers", &lib::spt::artist::followers)),
			decltype(orm::make_column("popularity", &lib::spt::artist::popularity)),
			decltype(orm::make_column("image", &lib::spt::artist::image))>;

		using tracks = orm::internal::table_t<lib::spt::track,
			decltype(orm::make_column("id", &lib::spt::track::id, orm::primary_key())),
			decltype(orm::make_column("name", &lib::spt::track::name)),
			decltype(orm::make_column("is_local", &lib::spt::track::is_local)),
			decltype(orm::make_column("is_playable", &lib::spt::track::is_playable)),
			decltype(orm::make_column("duration", &lib::spt::track::duration)),
			decltype(orm::make_column("added_at", &lib::spt::track::added_at)),
			decltype(orm::make_column("image", &lib::spt::track::image))>;

		using images = orm::internal::table_t<lib::db::image,
			decltype(orm::make_column("url", &lib::db::image::url, orm::primary_key())),
			decltype(orm::make_column("data", &lib::db::image::data))>;

		using storage = orm::internal::storage_t<albums, artists, tracks,
			images>;
	}
}
