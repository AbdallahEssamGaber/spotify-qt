#pragma once

#include "thirdparty/filesystem.hpp"
#include "thirdparty/json.hpp"
#include "lib/paths/paths.hpp"
#include "lib/format.hpp"
#include "lib/log.hpp"
#include "lib/json.hpp"
#include "lib/spotify/track.hpp"
#include "lib/spotify/playlist.hpp"
#include "lib/spotify/album.hpp"

namespace lib
{
	/**
	 * Handles cache
	 */
	class cache
	{
	public:
		/**
		 * Instance a new cache manager, does not create any directories
		 * @param paths Paths to get cache directory
		 */
		explicit cache(const paths &paths);

		//region album

		/**
		 * Get album image data
		 * @param id Album ID
		 * @return Binary JPEG data, or an empty vector if none
		 */
		std::vector<unsigned char> get_album_image(const std::string &url);

		/**
		 * Set album image data
		 * @param id Album ID
		 * @param data Binary JPEG data to save
		 */
		void set_album_image(const std::string &url, const std::vector<unsigned char> &data);

		//endregion

		//region playlists

		/**
		 * Get list of user's playlists
		 */
		std::vector<lib::spt::playlist> get_playlists();

		/**
		 * Set list of user's playlists
		 */
		void set_playlists(const std::vector<spt::playlist> &playlists);

		//endregion

		//region playlist

		/**
		 * Loads tracks from a cached playlist
		 * @param id Playlist ID
		 * @return Tracks or an empty vector on failure
		 */
		lib::spt::playlist get_playlist(const std::string &id);

		/**
		 * Save playlist to cache
		 * @param playlist Playlist to save
		 */
		void set_playlist(const spt::playlist &playlist);

		//endregion

		//region tracks

		/**
		 * Get tracks saved in cache
		 * @param id Id of album for example
		 * @return JSON stored in cache, or an empty object if none
		 */
		std::vector<lib::spt::track> tracks(const std::string &id);

		/**
		 * Save tracks to cache
		 * @param id Id of album for example
		 * @param tracks Tracks to save
		 */
		void tracks(const std::string &id, const std::vector<lib::spt::track> &tracks);

		/**
		 * Get all tracks saved in cache
		 * @return Map as id: tracks
		 */
		std::map<std::string, std::vector<lib::spt::track>> all_tracks();

		//endregion

	private:
		const lib::paths &paths;

		/**
		 * Get parent directory for cache type
		 */
		ghc::filesystem::path dir(const std::string &type);

		/**
		 * Get file name for id
		 */
		static std::string file(const std::string &id, const std::string &extension);

		/**
		 * Get full file path for cache type and id
		 */
		ghc::filesystem::path path(const std::string &type, const std::string &id,
			const std::string &extension);

		static std::string get_url_id(const ghc::filesystem::path &path);
	};
}
