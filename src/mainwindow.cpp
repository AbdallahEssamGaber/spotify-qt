#include "mainwindow.hpp"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
	// Some default values to prevent unexpected stuff
	playlists 	= nullptr;
	songs 		= nullptr;
	sptClient	= nullptr;
	volume 		= progress	= nullptr;
	nowPlaying	= position	= nowAlbum	= nullptr;
	repeat 		= shuffle	= playPause	= nullptr;
	// Set cache root location
	cacheLocation = QStandardPaths::standardLocations(QStandardPaths::CacheLocation)[0];
	// Create main cache path and album subdir
	QDir cacheDir(cacheLocation);
	cacheDir.mkpath(".");
	cacheDir.mkdir("album");
	cacheDir.mkdir("playlist");
	// Set Spotify
	spotify = new spt::Spotify();
	sptPlaylists = new QVector<spt::Playlist>();
	network = new QNetworkAccessManager();
	// Setup main window
	setWindowTitle("spotify-qt");
	setWindowIcon(icon("logo:spotify"));
	resize(1280, 720);
	setCentralWidget(createCentralWidget());
	addToolBar(Qt::ToolBarArea::TopToolBarArea, createToolBar());
	// Apply selected style
	Settings settings;
	QApplication::setStyle(settings.style());
	// Update player status
	auto timer = new QTimer(this);
	QTimer::connect(timer, &QTimer::timeout, this, &MainWindow::refresh);
	refresh();
	timer->start(1000);
	setStatus("Welcome to spotify-qt!");
	// Check if should start client
	if (settings.sptStartClient())
	{
		sptClient = new spt::ClientHandler();
		auto status = sptClient->start();
		if (!status.isEmpty())
			QMessageBox::warning(this,
				"Client error",
				QString("Failed to autostart Spotify client: %1").arg(status));
	}
}

MainWindow::~MainWindow()
{
	delete	playlists;
	delete	songs;
	delete	nowPlaying;
	delete	position;
	delete	nowAlbum;
	delete	progress;
	delete	playPause;
	delete	sptPlaylists;
	delete	spotify;
	delete	sptClient;
}

void MainWindow::refresh()
{
	current = spotify->currentPlayback();
	if (!current.isPlaying)
	{
		playPause->setIcon(icon("media-playback-start"));
		playPause->setText("Play");
		return;
	}
	auto currPlaying = QString("%1\n%2").arg(current.item->name).arg(current.item->artist);
	if (nowPlaying->text() != currPlaying)
	{
		if (nowPlaying->text() != "No music playing")
			setCurrentSongIcon();
		nowPlaying->setText(currPlaying);
		setAlbumImage(current.item->image);
		setWindowTitle(QString("%1 - %2").arg(current.item->artist).arg(current.item->name));
	}
	position->setText(QString("%1/%2")
		.arg(formatTime(current.progressMs))
		.arg(formatTime(current.item->duration)));
	progress->setValue(current.progressMs);
	progress->setMaximum(current.item->duration);
	playPause->setIcon(icon(
		current.isPlaying ? "media-playback-pause" : "media-playback-start"));
	playPause->setText(current.isPlaying ? "Pause" : "Play");
	if (!Settings().pulseVolume())
		volume->setValue(current.volume / 5);
	repeat->setChecked(current.repeat != "off");
	shuffle->setChecked(current.shuffle);
}

QGroupBox *createGroupBox(QVector<QWidget*> &widgets)
{
	auto group = new QGroupBox();
	auto layout = new QVBoxLayout();
	for (auto &widget : widgets)
		layout->addWidget(widget);
	group->setLayout(layout);
	return group;
}

QWidget *layoutToWidget(QLayout *layout)
{
	auto widget = new QWidget();
	widget->setLayout(layout);
	return widget;
}

QWidget *MainWindow::createCentralWidget()
{
	auto container = new QSplitter();
	// Sidebar with playlists etc.
	auto sidebar = new QVBoxLayout();
	auto libraryList = new QListWidget();
	playlists = new QListWidget();
	// Library
	libraryList->addItems({
		"Made For You", "Recently Played", "Liked Songs", "Albums", "Artists"
	});
	QListWidget::connect(libraryList, &QListWidget::itemPressed, this, [=](QListWidgetItem *item) {
		if (item != nullptr) {
			playlists->setCurrentRow(-1);
		}
	});
	libraryList->setEnabled(false);
	libraryList->setToolTip("Not implemented yet");
	auto library = createGroupBox(QVector<QWidget*>() << libraryList);
	library->setTitle("Library");
	sidebar->addWidget(library);
	// Update current playlists
	refreshPlaylists();
	// Set default selected playlist
	playlists->setCurrentRow(0);
	QListWidget::connect(playlists, &QListWidget::itemClicked, this, [this, libraryList](QListWidgetItem *item) {
		if (item != nullptr)
			libraryList->setCurrentRow(-1);
		auto currentPlaylist = sptPlaylists->at(playlists->currentRow());
		loadPlaylist(currentPlaylist);
	});
	QListWidget::connect(playlists, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
		auto currentPlaylist = sptPlaylists->at(playlists->currentRow());
		loadPlaylist(currentPlaylist);
		auto result = spotify->playTracks(
			QString("spotify:playlist:%1").arg(currentPlaylist.id));
		if (!result.isEmpty())
			setStatus(QString("Failed to start playlist playback: %1").arg(result));
	});
	auto playlistContainer = createGroupBox(QVector<QWidget*>() << playlists);
	playlistContainer->setTitle("Playlists");
	sidebar->addWidget(playlistContainer);
	// Now playing song
	auto nowPlayingLayout = new QHBoxLayout();
	nowPlayingLayout->setSpacing(12);
	nowAlbum = new QLabel();
	nowAlbum->setFixedSize(64, 64);
	nowAlbum->setPixmap(icon("media-optical-audio").pixmap(nowAlbum->size()));
	nowPlayingLayout->addWidget(nowAlbum);
	nowPlaying = new QLabel("No music playing");
	nowPlaying->setWordWrap(true);
	nowPlayingLayout->addWidget(nowPlaying);
	sidebar->addLayout(nowPlayingLayout);
	// Show menu when clicking now playing
	nowPlaying->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	QLabel::connect(nowPlaying, &QWidget::customContextMenuRequested, [this](const QPoint &pos) {
		auto track = current.item;
		songMenu(nowPlaying, track->id, track->artist, track->name, track->artistId, track->albumId)
			->popup(nowPlaying->mapToGlobal(pos));
	});
	// Sidebar as widget
	auto sidebarWidget = layoutToWidget(sidebar);
	sidebarWidget->setMaximumWidth(250);
	container->addWidget(sidebarWidget);
	// Table with songs
	songs = new QTreeWidget();
	songs->setEditTriggers(QAbstractItemView::NoEditTriggers);
	songs->setSelectionBehavior(QAbstractItemView::SelectRows);
	songs->setSortingEnabled(true);
	songs->setRootIsDecorated(false);
	songs->setAllColumnsShowFocus(true);
	songs->setColumnCount(5);
	songs->setHeaderLabels({
		" ", "Title", "Artist", "Album", "Length", "Added"
	});
	songs->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	songs->header()->setSectionsMovable(false);
	// Song context menu
	songs->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	QWidget::connect(songs, &QWidget::customContextMenuRequested, [=](const QPoint &pos) {
		auto item = songs->itemAt(pos);
		auto trackId = item->data(0, RoleTrackId).toString();
		if (trackId.isEmpty())
			return;
		songMenu(songs, trackId, item->text(2), item->text(1),
			item->data(0, RoleArtistId).toString(),
			item->data(0, RoleAlbumId).toString())->popup(songs->mapToGlobal(pos));
	});
	QTreeWidget::connect(songs, &QTreeWidget::itemClicked, this, [=](QTreeWidgetItem *item, int column) {
		auto trackId = item->data(0, RoleTrackId).toString();
		if (trackId.isEmpty())
		{
			setStatus("Failed to start playback: track not found");
			return;
		}
		auto status = spotify->playTracks(trackId, sptContext);
		if (!status.isEmpty())
			setStatus(QString("Failed to start playback: %1").arg(status));
		refresh();
	});
	// Load tracks in playlist
	auto playlistId = Settings().lastPlaylist();
	// Default to first in list
	if (playlistId.isEmpty())
		playlistId = sptPlaylists->at(0).id;
	// Find playlist in list
	int i = 0;
	for (auto &playlist : *sptPlaylists)
	{
		if (playlist.id == playlistId)
		{
			playlists->setCurrentRow(i);
			loadPlaylist(playlist);
		}
		i++;
	}
	// Add to main thing
	container->addWidget(songs);
	return container;
}

QMenu *MainWindow::songMenu(QWidget *parent, const QString &trackId, const QString &artist,
	const QString &name, const QString &artistId, const QString &albumId)
{
	auto songMenu = new QMenu(parent);
	auto trackFeatures = songMenu->addAction(icon("view-statistics"), "Audio features");
	QAction::connect(trackFeatures, &QAction::triggered, [=](bool checked) {
		openAudioFeaturesWidget(trackId, artist, name);
	});
	auto lyrics = songMenu->addAction(icon("view-media-lyrics"), "Lyrics");
	QAction::connect(lyrics, &QAction::triggered, [=](bool checked) {
		openLyrics(artist, name);
	});
	auto share = songMenu->addMenu(icon("document-share"), "Share");
	auto shareSongLink = share->addAction("Copy song link");
	QAction::connect(shareSongLink, &QAction::triggered, [=](bool checked) {
		QApplication::clipboard()->setText(
			QString("https://open.spotify.com/track/%1")
				.arg(QString(trackId).remove(0, QString("spotify:track:").length())));
		setStatus("Link copied to clipboard");
	});
	songMenu->addSeparator();
	// Add to playlist
	auto addPlaylist = songMenu->addMenu(icon("list-add"), "Add to playlist");
	auto currentPlaylist = playlists->currentRow() == -1
		? nullptr : &sptPlaylists->at(playlists->currentRow());
	for (auto &playlist : *sptPlaylists)
	{
		// Create main action
		auto action = addPlaylist->addAction(playlist.name);
		action->setData(playlist.id);
	}
	QMenu::connect(addPlaylist, &QMenu::triggered, [this, trackId](QAction *action) {
		// Check if it's already in the playlist
		auto playlistId = action->data().toString();
		spt::Playlist *playlist = nullptr;
		for (auto &pl : *sptPlaylists)
			if (pl.id == playlistId)
			{
				if (QMessageBox::information(this,
					"Duplicate",
					"Track is already in the playlist, do you want to add it anyway?",
					QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No)
					return;
				break;
			}
		// Actually add
		auto result = spotify->addToPlaylist(playlistId, trackId);
		if (!result.isEmpty())
			setStatus(QString("Failed to add track to playlist: %1").arg(result));
	});
	// Remove from playlist
	auto remPlaylist = songMenu->addAction(icon("list-remove"), "Remove from playlist");
	QAction::connect(remPlaylist, &QAction::triggered, [this, trackId, name, artist, currentPlaylist](bool checked) {
		// Remove from interface
		for (int i = 0; i < songs->topLevelItemCount(); i++)
		{
			auto item = songs->topLevelItem(i);
			if (item->data(0, RoleTrackId).toString() == trackId)
			{
				songs->takeTopLevelItem(i);
				break;
			}
		}
		// Remove from Spotify
		// TODO
		// Update status
		setStatus(QString("Removed \"%1 - %2\" from \"%3\"")
			.arg(name).arg(artist).arg(currentPlaylist->name));
	});
	songMenu->addSeparator();
	auto goArtist = songMenu->addAction(icon("view-media-artist"), "View artist");
	QAction::connect(goArtist, &QAction::triggered, [=](bool checked) {
		openArtist(artistId);
	});
	auto goAlbum = songMenu->addAction(icon("view-media-album-cover"), "Open album");
	goAlbum->setEnabled(!sptContext.startsWith("spotify:album"));
	QAction::connect(goAlbum, &QAction::triggered, [=](bool checked) {
		loadAlbum(albumId);
	});
	return songMenu;
}

QToolBar *MainWindow::createToolBar()
{
	auto toolBar = new QToolBar(this);
	toolBar->setMovable(false);
	// Menu
	auto menu = new QToolButton(this);
	menu->setText("Menu");
	menu->setIcon(icon("application-menu"));
	menu->setPopupMode(QToolButton::InstantPopup);
	menu->setMenu(createMenu());
	toolBar->addWidget(menu);
	toolBar->addSeparator();
	// Media controls
	auto previous = toolBar->addAction(icon("media-skip-backward"), "Previous");
	playPause = toolBar->addAction(icon("media-playback-start"), "Play");
	QAction::connect(playPause, &QAction::triggered, [=](bool checked) {
		auto status = playPause->iconText() == "Pause" ? spotify->pause() : spotify->resume();
		if (!status.isEmpty())
		{
			setStatus(QString("Failed to %1 playback: %2")
				.arg(playPause->iconText() == "Pause" ? "pause" : "resume")
				.arg(status));
		}
	});
	auto next = toolBar->addAction(icon("media-skip-forward"),  "Next");
	QAction::connect(previous, &QAction::triggered, [=](bool checked) {
		auto status = spotify->previous();
		if (!status.isEmpty())
			setStatus(QString("Failed to go to previous track: %1").arg(status));
		refresh();
	});
	QAction::connect(next, &QAction::triggered, [=](bool checked) {
		auto status = spotify->next();
		if (!status.isEmpty())
			setStatus(QString("Failed to go to next track: %1").arg(status));
		refresh();
	});
	// Progress
	progress = new QSlider(this);
	progress->setOrientation(Qt::Orientation::Horizontal);
	QSlider::connect(progress, &QAbstractSlider::sliderReleased, [=]() {
		auto status = spotify->seek(progress->value());
		if (!status.isEmpty())
			setStatus(QString("Failed to seek: %1").arg(status));
	});
	toolBar->addSeparator();
	toolBar->addWidget(progress);
	toolBar->addSeparator();
	position = new QLabel("0:00/0:00", this);
	toolBar->addWidget(position);
	toolBar->addSeparator();
	// Shuffle and repeat toggles
	shuffle = toolBar->addAction(icon("media-playlist-shuffle"), "Shuffle");
	shuffle->setCheckable(true);
	QAction::connect(shuffle, &QAction::triggered, [=](bool checked) {
		auto status = spotify->setShuffle(checked);
		if (!status.isEmpty())
			setStatus(QString("Failed to toggle shuffle: %1").arg(status));
		refresh();
	});
	repeat = toolBar->addAction(icon("media-playlist-repeat"), "Repeat");
	repeat->setCheckable(true);
	QAction::connect(repeat, &QAction::triggered, [=](bool checked) {
		auto status = spotify->setRepeat(checked ? "context" : "off");
		if (!status.isEmpty())
			setStatus(QString("Failed to toggle repeat: %1").arg(status));
		refresh();
	});
	// Volume slider
	volume = new QSlider(this);
	volume->setOrientation(Qt::Orientation::Horizontal);
	volume->setMaximumWidth(100);
	volume->setMinimum(0);
	volume->setMaximum(20);
	volume->setValue(20);
	toolBar->addWidget(volume);
	Settings settings;
	if (settings.pulseVolume())
	{
		// If using PulseAudio for volume control, update on every
		QSlider::connect(volume, &QAbstractSlider::valueChanged, [](int value) {
			QProcess process;
			// Find what sink to use
			process.start("/usr/bin/pactl", {
				"list", "sink-inputs"
			});
			process.waitForFinished();
			auto sinks = QString(process.readAllStandardOutput()).split("Sink Input #");
			QString sink;
			for (auto &s : sinks)
				if (s.contains("Spotify"))
					sink = s;
			if (sink.isEmpty())
				return;
			// Sink was found, get id
			auto left = sink.left(sink.indexOf('\n'));
			auto sinkId = left.right(left.length() - left.lastIndexOf('#') - 1);
			process.start("/usr/bin/pactl", {
				"set-sink-input-volume", sinkId, QString::number(value * 0.05, 'f', 2)
			});
			process.waitForFinished();
		});
	}
	else
	{
		// If using Spotify for volume control, only update on release
		QSlider::connect(volume, &QAbstractSlider::sliderReleased, [=]() {
			auto status = spotify->setVolume(volume->value() * 5);
			if (!status.isEmpty())
				setStatus(QString("Failed to set volume: %1").arg(status));
		});
	}
	// Return final tool bar
	return toolBar;
}

QAction *MainWindow::createMenuAction(const QString &iconName,
	const QString &text, QKeySequence::StandardKey shortcut)
{
	auto action = new QAction(icon(iconName), text);
	if (shortcut != QKeySequence::UnknownKey)
		action->setShortcut(QKeySequence(shortcut));
	return action;
}

void MainWindow::refreshDevices(QMenu *deviceMenu)
{
	// Set status and get devices
	setStatus("Refreshing devices...");
	auto devices = spotify->devices();
	// Clear all entries
	for (auto &action : deviceMenu->actions())
		deviceMenu->removeAction(action);
	// Check if empty
	if (devices.isEmpty())
	{
		setStatus("No devices found");
		deviceMenu->addAction("No devices found")->setDisabled(true);
		return;
	}
	// Update devices
	setStatus(QString("Found %1 device(s)").arg(devices.length()));
	for (auto &device : devices)
	{
		auto action = deviceMenu->addAction(device.name);
		action->setCheckable(true);
		action->setChecked(device.isActive);
		action->setDisabled(device.isActive);
		QAction::connect(action, &QAction::triggered, [=](bool triggered) {
			auto status = spotify->setDevice(device);
			if (!status.isEmpty())
			{
				action->setChecked(false);
				setStatus(QString("Failed to set device: %1").arg(status));
			}
			else
				action->setDisabled(true);
		});
	}
}

QTreeWidgetItem *treeItem(QTreeWidget *tree, const QString &key, const QString &value)
{
	return new QTreeWidgetItem(tree, {
		key, value
	});
}

void MainWindow::openAudioFeaturesWidget(const QString &trackId, const QString &artist, const QString &name)
{
	auto features = spotify->trackAudioFeatures(trackId);
	auto dock = new QDockWidget(QString("%1 - %2")
		.arg(artist)
		.arg(name)
		.replace("&", "&&"), this);
	auto tree = new QTreeWidget(dock);
	tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tree->header()->hide();
	tree->setSelectionMode(QAbstractItemView::NoSelection);
	tree->setRootIsDecorated(false);
	tree->setAllColumnsShowFocus(true);
	tree->setColumnCount(2);
	tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	QMapIterator<QString, QString> i(features.values);
	while (i.hasNext())
	{
		i.next();
		tree->addTopLevelItem(treeItem(tree, i.key(), i.value()));
	}
	dock->setWidget(tree);
	dock->setMinimumWidth(150);
	addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dock);
}

void MainWindow::openLyrics(const QString &artist, const QString &name)
{
	setStatus("Loading lyrics...");
	auto reply = network->get(QNetworkRequest(QUrl(QString(
		"https://vscode-spotify-lyrics.azurewebsites.net/lyrics?artist=%1&title=%2")
			.arg(artist)
			.arg(name))));
	while (!reply->isFinished())
		QCoreApplication::processEvents();
	auto lyricsText = QString(reply->readAll()).trimmed();
	if (lyricsText == "Not found")
	{
		setStatus("Lyrics not found");
		return;
	}
	setStatus("Lyrics loaded");
	auto dock = new QDockWidget(
		QString("%1 - %2")
			.arg(artist)
			.arg(name), this);
	auto lyricsView = new QTextEdit(dock);
	lyricsView->setHtml(lyricsText.replace("\n", "<br/>"));
	lyricsView->setReadOnly(true);
	dock->setWidget(lyricsView);
	dock->setMinimumWidth(300);
	addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dock);
}

QMenu *MainWindow::createMenu()
{
	// Create root
	auto menu = new QMenu(this);
	// About
	auto aboutMenu = new QMenu("About");
	auto aboutQt = createMenuAction("logo:qt", "About Qt", QKeySequence::UnknownKey);
	QAction::connect(aboutQt, &QAction::triggered, [=](bool checked) {
		QMessageBox::aboutQt(this);
	});
	auto checkForUpdates = createMenuAction("download", "Check for updates", QKeySequence::UnknownKey);
	QAction::connect(checkForUpdates, &QAction::triggered, [=](bool checked) {
		setStatus("Checking for updates...");
		auto json = getJson("https://api.github.com/repos/kraxarn/spotify-qt/releases");
		auto latest = json.array()[0].toObject()["tag_name"].toString();
		setStatus(latest == APP_VERSION
				  ? "You have the latest version"
				  : QString("Update found, latest version is %1, you have version %2")
					  .arg(latest)
					  .arg(APP_VERSION));
	});
	aboutMenu->setIcon(icon("help-about"));
	aboutMenu->addAction(QString("spotify-qt %1").arg(APP_VERSION))->setDisabled(true);
	aboutMenu->addActions({
		aboutQt, checkForUpdates
	});
	aboutMenu->addSeparator();
	QAction::connect(
		aboutMenu->addAction(icon("folder-temp"), "Open cache directory"),
		&QAction::triggered, [this](bool checked) {
			if (!QDesktopServices::openUrl(QUrl(cacheLocation)))
				QMessageBox::warning(this,
					"No path",
					QString("Failed to open path: %1").arg(cacheLocation));
		}
	);
	QAction::connect(
		aboutMenu->addAction(icon("folder-txt"), "Open config file"),
		&QAction::triggered, [this](bool checked) {
			if (!QDesktopServices::openUrl(QUrl(Settings().fileName())))
				QMessageBox::warning(this,
					"No file",
					QString("Failed to open file: %1").arg(Settings().fileName()));
		}
	);
	menu->addMenu(aboutMenu);
	// Device selection
	auto deviceMenu = new QMenu("Device");
	deviceMenu->setIcon(icon("speaker"));
	QMenu::connect(deviceMenu, &QMenu::aboutToShow, [=]() {
		refreshDevices(deviceMenu);
	});
	menu->addMenu(deviceMenu);
	// Refresh and settings
	auto openSettings = createMenuAction("settings", "Settings...", QKeySequence::Preferences);
	QAction::connect(openSettings, &QAction::triggered, [=]() {
		SettingsDialog dialog(this);
		dialog.exec();
	});
	menu->addAction(openSettings);
	// Debug stuff
	menu->addSeparator();
	auto refPlaylist = menu->addAction(
		QIcon::fromTheme("reload"), "Refresh current playlist");
	QAction::connect(refPlaylist, &QAction::triggered, [&]() {
		auto currentPlaylist = sptPlaylists->at(playlists->currentRow());
		refreshPlaylist(currentPlaylist, true);
	});
	// Log out and quit
	menu->addSeparator();
	auto quitAction = createMenuAction("application-exit", "Quit", QKeySequence::Quit);
	QAction::connect(quitAction, &QAction::triggered, QCoreApplication::quit);
	auto logOutAction = createMenuAction("im-user-away",  "Log out", QKeySequence::UnknownKey);
	QAction::connect(logOutAction, &QAction::triggered, [this](){
		QMessageBox box(
			QMessageBox::Icon::Question,
			"Are you sure?",
			"Do you also want to clear your application credentials or only log out?");
		auto clearAll = box.addButton("Clear everything", QMessageBox::ButtonRole::AcceptRole);
		auto logOut   = box.addButton("Only log out",     QMessageBox::ButtonRole::AcceptRole);
		auto cancel   = box.addButton("Cancel",           QMessageBox::ButtonRole::RejectRole);
		box.exec();
		auto result = box.clickedButton();
		// Return if we pressed cancel
		if (result == nullptr || result == cancel)
			return;
		Settings settings;
		// Clear client secret/id if clearAll
		if (result == clearAll)
			settings.removeClient();
		// Clear login if cleatAll/logOut
		if (result == clearAll || result == logOut)
			settings.removeTokens();
		QMessageBox::information(this,
			"Logged out",
			"You are now logged out, the application will now close");
		QCoreApplication::quit();
	});
	menu->addActions({
		logOutAction, quitAction
	});

	// Return final menu
	return menu;
}

void MainWindow::refreshPlaylists()
{
	spotify->playlists(&sptPlaylists);
	// Create or empty
	if (playlists == nullptr)
		playlists = new QListWidget();
	else
		playlists->clear();
	// Add all playlists
	for (auto &playlist : *sptPlaylists)
		playlists->addItem(playlist.name);
}

bool MainWindow::loadSongs(const QVector<spt::Track> &tracks)
{
	songs->clear();
	for (int i = 0; i < tracks.length(); i++)
	{
		auto track = tracks.at(i);
		auto item = new QTreeWidgetItem({
			"", track.name, track.artist, track.album,
			formatTime(track.duration), track.addedAt.toString("yyyy-MM-dd")
		});
		item->setData(0, RoleTrackId,  QString("spotify:track:%1").arg(track.id));
		item->setData(0, RoleArtistId, track.artistId);
		item->setData(0, RoleAlbumId,  track.albumId);
		if (track.isLocal)
		{
			item->setDisabled(true);
			item->setToolTip(1, "Local track");
		}
		if (current.item != nullptr && track.id == current.item->id)
			item->setIcon(0, icon("media-playback-start"));
		songs->insertTopLevelItem(i, item);
	}
	return true;
}

bool MainWindow::loadAlbum(const QString &albumId, bool ignoreEmpty)
{
	auto tracks = spotify->albumTracks(albumId);
	if (ignoreEmpty && tracks->length() <= 1)
		setStatus("Album only contains one song or is empty");
	else
	{
		playlists->setCurrentRow(-1);
		sptContext = QString("spotify:album:%1").arg(albumId);
		loadSongs(*tracks);
	}
	delete tracks;
	return true;
}

bool MainWindow::loadPlaylist(spt::Playlist &playlist)
{
	Settings().setLastPlaylist(playlist.id);
	if (loadPlaylistFromCache(playlist))
		return true;
	songs->setEnabled(false);
	auto result = loadSongs(playlist.loadTracks(*spotify));
	songs->setEnabled(true);
	sptContext = QString("spotify:playlist:%1").arg(playlist.id);
	if (result)
		cachePlaylist(playlist);
	return result;
}

bool MainWindow::loadPlaylistFromCache(spt::Playlist &playlist)
{
	auto tracks = playlistTracks(playlist.id);
	if (tracks.isEmpty())
		return false;
	songs->setEnabled(false);
	auto result = loadSongs(tracks);
	songs->setEnabled(true);
	sptContext = QString("spotify:playlist:%1").arg(playlist.id);
	refreshPlaylist(playlist);
	return result;
}

QVector<spt::Track> MainWindow::playlistTracks(const QString &playlistId)
{
	QVector<spt::Track> tracks;
	auto filePath = QString("%1/playlist/%2").arg(cacheLocation).arg(playlistId);
	if (!QFileInfo::exists(filePath))
		return tracks;
	QFile file(filePath);
	file.open(QIODevice::ReadOnly);
	auto json = QJsonDocument::fromBinaryData(file.readAll(), QJsonDocument::BypassValidation);
	if (json.isNull())
		return tracks;
	for (auto track : json["tracks"].toArray())
		tracks.append(spt::Track(track.toObject()));
	return tracks;
}

void MainWindow::refreshPlaylist(spt::Playlist &playlist, bool force)
{
	auto newPlaylist = spotify->playlist(playlist.id);
	if (!force && playlist.snapshot == newPlaylist.snapshot)
		return;
	loadSongs(newPlaylist.loadTracks(*spotify));
	cachePlaylist(newPlaylist);
}

void MainWindow::setStatus(const QString &message)
{
	statusBar()->showMessage(message, 5000);
}

void MainWindow::setCurrentSongIcon()
{
	for (int i = 0; i < songs->topLevelItemCount(); i++)
	{
		auto item = songs->topLevelItem(i);
		if (item->data(0, RoleTrackId).toString() == QString("spotify:track:%1").arg(current.item->id))
			item->setIcon(0, icon("media-playback-start"));
		else
			item->setIcon(0, QIcon());
	}
}

void MainWindow::setAlbumImage(const QString &url)
{
	nowAlbum->setPixmap(getAlbum(url));
}

QString MainWindow::formatTime(int ms)
{
	auto duration = QTime(0, 0).addMSecs(ms);
	return QString("%1:%2")
		.arg(duration.minute())
		.arg(duration.second() % 60, 2, 10, QChar('0'));
}

QByteArray MainWindow::get(const QString &url)
{
	auto reply = network->get(QNetworkRequest(QUrl(url)));
	while (!reply->isFinished())
		QCoreApplication::processEvents();
	reply->deleteLater();
	return reply->readAll();
}

QJsonDocument MainWindow::getJson(const QString &url)
{
	return QJsonDocument::fromJson(get(url));
}

QPixmap MainWindow::getAlbum(const QString &url)
{
	QPixmap img;
	// Check if cache exists
	auto cachePath = QString("%1/album/%2").arg(cacheLocation).arg(QFileInfo(url).baseName());
	if (QFileInfo::exists(cachePath))
	{
		// Read file from cache
		QFile file(cachePath);
		file.open(QIODevice::ReadOnly);
		img.loadFromData(file.readAll(), "jpeg");
	}
	else
	{
		// Download image and save to cache
		img.loadFromData(get(url), "jpeg");
		if (!img.save(cachePath, "jpeg"))
			qDebug() << "failed to save album cache to" << cachePath;
	}
	return img;
}

void MainWindow::openArtist(const QString &artistId)
{
	auto artist = spotify->artist(artistId);
	auto dock = new QDockWidget(this);
	dock->setFeatures(QDockWidget::DockWidgetClosable);
	auto layout = new QVBoxLayout();
	layout->setContentsMargins(-1, 0, -1, 0);
	// Get cover image (320x320 -> 320x160)
	QPixmap cover;
	cover.loadFromData(get(artist.image), "jpeg");
	auto coverLabel = new QLabel(dock);
	coverLabel->setPixmap(cover.copy(0, 80, 320, 160));
	layout->addWidget(coverLabel);
	// Artist name title
	auto title = new QLabel(artist.name, dock);
	title->setAlignment(Qt::AlignHCenter);
	title->setWordWrap(true);
	auto titleFont = title->font();
	titleFont.setPointSize(24);
	title->setFont(titleFont);
	layout->addWidget(title);
	// Genres
	auto genres = new QLabel(artist.genres.join(", "));
	genres->setWordWrap(true);
	layout->addWidget(genres);
	// Tabs
	auto tabs = new QTabWidget(dock);
	layout->addWidget(tabs);
	// Top tracks
	auto topTracks = artist.topTracks(*spotify);
	QStringList topTrackIds;
	auto topTracksList = new QListWidget(tabs);
	for (auto &track : topTracks)
	{
		auto item = new QListWidgetItem(track.name, topTracksList);
		item->setIcon(QIcon(getAlbum(track.image)));
		item->setData(RoleTrackId, track.id);
		topTrackIds.append(QString("spotify:track:%1").arg(track.id));
	}
	QListWidget::connect(topTracksList, &QListWidget::itemClicked, [this, topTrackIds](QListWidgetItem *item) {
		auto result =  spotify->playTracks(
			QString("spotify:track:%1").arg(item->data(RoleTrackId).toString()), topTrackIds);
		if (!result.isEmpty())
			setStatus(QString("Failed to start playback: %1").arg(result));
	});
	tabs->addTab(topTracksList, "Popular");
	// Albums
	auto albums = artist.albums(*spotify);
	auto albumList = new QListWidget(tabs);
	auto singleList = new QListWidget(tabs);
	for (auto &album : albums)
	{
		auto parent = album.albumGroup == "single" ? singleList : albumList;
		auto year = album.releaseDate.toString("yyyy");
		auto item = new QListWidgetItem(QString("%1 (%2)")
			.arg(album.name)
			.arg(year.isEmpty() ? "unknown" : year), parent);
		item->setIcon(QIcon(getAlbum(album.image)));
		item->setData(RoleAlbumId, album.id);
	}
	auto loadAlbumId = [this](QListWidgetItem *item) {
		if (!loadAlbum(item->data(RoleAlbumId).toString(), false))
			setStatus(QString("Failed to load album"));
	};
	QListWidget::connect(albumList,  &QListWidget::itemClicked, loadAlbumId);
	QListWidget::connect(singleList, &QListWidget::itemClicked, loadAlbumId);
	tabs->addTab(albumList,  "Albums");
	tabs->addTab(singleList, "Singles");
	// Related artists
	auto relatedArtists = artist.relatedArtists(*spotify);
	auto relatedList = new QListWidget(tabs);
	for (auto &related : relatedArtists)
	{
		auto item = new QListWidgetItem(related.name, relatedList);
		item->setData(RoleArtistId, related.id);
	}
	QListWidget::connect(relatedList, &QListWidget::itemClicked, [this, relatedList, dock](QListWidgetItem *item) {
		relatedList->setEnabled(false);
		openArtist(item->data(RoleArtistId).toString());
		dock->close();
	});
	tabs->addTab(relatedList, "Related");
	// Rest of dock
	dock->setWidget(layoutToWidget(layout));
	dock->setFixedWidth(320);
	addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dock);
}

void MainWindow::cachePlaylist(spt::Playlist &playlist)
{
	QJsonDocument json(playlist.toJson(*spotify));
	QFile file(QString("%1/playlist/%2").arg(cacheLocation).arg(playlist.id));
	file.open(QIODevice::WriteOnly);
	file.write(json.toBinaryData());
}

QIcon MainWindow::icon(const QString &name)
{
	if (name.startsWith("logo:"))
	{
		QString icName = name.right(name.length() - QString("logo:").length());
		return QIcon::fromTheme(icName, QIcon(QString(":/res/logo/%1.svg").arg(icName)));
	}
	return QIcon::fromTheme(name, QIcon(QString(":/res/ic/%1.svg").arg(name)));
}