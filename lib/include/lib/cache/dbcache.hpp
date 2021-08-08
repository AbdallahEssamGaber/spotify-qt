#pragma once

#include "lib/cache.hpp"
#include "lib/cache/tables.hpp"
#include "lib/cache/types.hpp"

#include "lib/paths/paths.hpp"
#include "lib/stopwatch.hpp"

namespace lib
{
	class db_cache: public ::lib::cache
	{
	public:
		db_cache(const lib::paths &paths);

		auto get_album_image(const std::string &url) const -> std::vector<unsigned char> override;
		void set_album_image(const std::string &url,
			const std::vector<unsigned char> &data) override;

		auto get_playlists() const -> std::vector<lib::spt::playlist> override;
		void set_playlists(const std::vector<spt::playlist> &playlists) override;

		auto get_playlist(const std::string &id) const -> lib::spt::playlist override;
		void set_playlist(const spt::playlist &playlist) override;

		auto get_tracks(const std::string &id) const -> std::vector<lib::spt::track> override;
		void set_tracks(const std::string &id, const std::vector<lib::spt::track> &tracks) override;
		auto all_tracks() const -> std::map<std::string, std::vector<lib::spt::track>> override;

		auto get_track_info(const spt::track &track) const -> lib::spt::track_info override;
		void set_track_info(const spt::track &track, const spt::track_info &track_info) override;

		void add_crash(const crash_info &info) override;
		auto get_all_crashes() const -> std::vector<lib::crash_info> override;

	private:
		lib::db::storage storage;

		static auto make_storage(const lib::paths &paths) -> lib::db::storage;
	};
}
