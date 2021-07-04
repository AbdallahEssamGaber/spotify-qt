#pragma once

#include "mainwindow.hpp"
#include "spotify/spotify.hpp"
#include "lib/httpclient.hpp"

#include "searchtab/tracks.hpp"
#include "searchtab/artists.hpp"
#include "searchtab/albums.hpp"
#include "searchtab/playlists.hpp"

#include <QDockWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QTabWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

class SearchView: public QWidget
{
Q_OBJECT

public:
	SearchView(spt::Spotify &spotify, lib::cache &cache,
		const lib::http_client &httpClient, QWidget *parent);

private:
	QTabWidget *tabs = nullptr;
	QLineEdit *searchBox = nullptr;
	QListWidget *playlistList = nullptr;
	spt::Spotify &spotify;
	lib::cache &cache;
	const lib::http_client &httpClient;

	SearchTab::Tracks *tracks = nullptr;
	SearchTab::Artists *artists = nullptr;
	SearchTab::Albums *albums = nullptr;
	SearchTab::Playlists *playlists = nullptr;

	auto defaultTree(const QStringList &headers) -> QTreeWidget *;
	void search();

protected:
	void showEvent(QShowEvent *event) override;
	void hideEvent(QHideEvent *event) override;

private:
	void addPlaylist(const lib::spt::playlist &playlist);

	void resultsLoaded(const lib::spt::search_results &results);
};
