#include "lib/cache/dbcache.hpp"

lib::db_cache::db_cache(const lib::paths &paths)
	: storage(make_storage(paths))
{
	lib::stopwatch stopwatch;
	stopwatch.start();

	storage.sync_schema();

	stopwatch.stop();
	lib::log::dev("Synced schema in {} ms",
		stopwatch.elapsed<lib::stopwatch::ms, long>());
}

auto lib::db_cache::make_storage(const lib::paths &paths) -> lib::db::storage
{
	// Spotify entities

	auto albums = orm::make_table<lib::spt::album>("albums",
		orm::make_column("id", &lib::spt::album::id, orm::primary_key()),
		orm::make_column("name", &lib::spt::album::name),
		orm::make_column("album_group", &lib::spt::album::album_group),
		orm::make_column("image", &lib::spt::album::image),
		orm::make_column("artist", &lib::spt::album::artist),
		orm::make_column("release_date", &lib::spt::album::release_date));

	auto artists = orm::make_table<lib::spt::artist>("artists",
		orm::make_column("id", &lib::spt::artist::id, orm::primary_key()),
		orm::make_column("name", &lib::spt::artist::name),
		orm::make_column("followers", &lib::spt::artist::followers),
		orm::make_column("popularity", &lib::spt::artist::popularity),
		orm::make_column("image", &lib::spt::artist::image));

	auto tracks = orm::make_table<lib::spt::track>("tracks",
		orm::make_column("id", &lib::spt::track::id, orm::primary_key()),
		orm::make_column("name", &lib::spt::track::name),
		orm::make_column("is_local", &lib::spt::track::is_local),
		orm::make_column("is_playable", &lib::spt::track::is_playable),
		orm::make_column("duration", &lib::spt::track::duration),
		orm::make_column("added_at", &lib::spt::track::added_at),
		orm::make_column("image", &lib::spt::track::image));

	// Other

	auto images = orm::make_table<lib::db::image>("images",
		orm::make_column("url", &lib::db::image::url, orm::primary_key()),
		orm::make_column("data", &lib::db::image::data));

	return orm::make_storage(paths.cache() / "cache.db",
		albums, artists, tracks, images);
}

auto lib::db_cache::get_album_image(const std::string &url) const -> std::vector<unsigned char>
{
	return {};
}

void lib::db_cache::set_album_image(const std::string &url, const std::vector<unsigned char> &data)
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

std::vector<lib::spt::track> lib::db_cache::get_tracks(const std::string &id) const
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
