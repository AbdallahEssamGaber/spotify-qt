#include "lib/cache/dbcache.hpp"
#include "lib/stopwatch.hpp"

lib::db_cache::db_cache(const lib::paths &paths)
	: db(paths.cache() / "cache.db")
{
	lib::stopwatch stopwatch;
	stopwatch.start();

	make_storage();

	stopwatch.stop();
	lib::log::debug("Synced schema in {} ms",
		stopwatch.elapsed<lib::stopwatch::ms, long>());
}

void lib::db_cache::make_storage()
{
	//region Metadata

	db << "create table if not exists metadata (version int)";
	if (sql<int>("select count(*) from metadata") == 0)
	{
		db << "insert into metadata (version) values (?)"
			<< 1;
	}

	//endregion

	//region Albums

	db << "create table if not exists album"
		  "(id text primary key not null,"
		  "album_type text not null,"
		  "total_tracks integer not null,"
		  "href text not null,"
		  "name text not null,"
		  "release_date text not null,"
		  "release_date_precision text not null,"
		  "uri text not null)";

	db << "create table if not exists album_market"
		  "(album_id text not null,"
		  "country text not null,"
		  "foreign key (album_id) references album (id))";

	db << "create table if not exists album_external_url"
		  "(album_id text not null,"
		  "url text not null,"
		  "description text not null,"
		  "foreign key (album_id) references album (id))";

	db << "create table if not exists album_image"
		  "(album_id text not null,"
		  "url text not null,"
		  "width integer not null,"
		  "height integer not null,"
		  "data blob null,"
		  "foreign key (album_id) references album (id))";

	db << "create table if not exists album_restriction"
		  "(album_id text not null,"
		  "reason text not null,"
		  "foreign key (album_id) references album (id))";

	db << "create table if not exists album_artist"
		  "(album_id text not null,"
		  "artist_id text not null,"
		  "foreign key (album_id) references album (id),"
		  "foreign key (artist_id) references artist (id))";

	db << "create table if not exists album_track"
		  "(album_id text not null,"
		  "track_id text not null,"
		  "foreign key (album_id) references album (id),"
		  "foreign key (track_id) references track (id))";

	//endregion

	//region Artists

	db << "create table if not exists artist"
		  "(id text primary key not null,"
		  "name text not null,"
		  "href text not null,"
		  "uri text not null,"
		  "followers integer not null,"
		  "popularity integer not null)";

	db << "create table if not exists artist_external_url"
		  "(artist_id text not null,"
		  "url text not null,"
		  "description text not null,"
		  "foreign key (artist_id) references artist (id))";

	db << "create table if not exists artist_genre"
		  "(artist_id text not null,"
		  "genre text not null,"
		  "foreign key (artist_id) references artist (id))";

	db << "create table if not exists artist_image"
		  "(artist_id text not null,"
		  "url text not null,"
		  "width integer not null,"
		  "height integer not null,"
		  "data blob null,"
		  "foreign key (artist_id) references artist (id))";

	//endregion
}

void lib::db_cache::from_json(const lib::json_cache &json_cache)
{

}

auto lib::db_cache::get_album_image(const std::string &url) const -> std::vector<unsigned char>
{
	return {};
}

void lib::db_cache::set_album_image(const std::string &url, const std::vector<unsigned char> &data)
{
}

std::string lib::db_cache::get_album_image_path(const std::string &url) const
{
	return {};
}

lib::spt::album lib::db_cache::get_album(const std::string &album_id) const
{
	return {};
}

void lib::db_cache::set_album(const lib::spt::album &album)
{
}

auto lib::db_cache::get_playlists() const -> std::vector<lib::spt::playlist>
{
	return {};
}

void lib::db_cache::set_playlists(const std::vector<spt::playlist> &playlists)
{

}

auto lib::db_cache::get_playlist(const std::string &id) const -> lib::spt::playlist
{
	return {};
}

void lib::db_cache::set_playlist(const lib::spt::playlist &playlist)
{

}

auto lib::db_cache::get_tracks(const std::string &id) const -> std::vector<lib::spt::track>
{
	return {};
}

void lib::db_cache::set_tracks(const std::string &id, const std::vector<lib::spt::track> &tracks)
{

}

auto lib::db_cache::all_tracks() const -> std::map<std::string, std::vector<lib::spt::track>>
{
	return {};
}

auto lib::db_cache::get_track_info(const lib::spt::track &track) const -> lib::spt::track_info
{
	return {};
}

void lib::db_cache::set_track_info(const lib::spt::track &track,
	const lib::spt::track_info &track_info)
{

}

void lib::db_cache::add_crash(const lib::crash_info &info)
{

}

auto lib::db_cache::get_all_crashes() const -> std::vector<lib::crash_info>
{
	return {};
}
