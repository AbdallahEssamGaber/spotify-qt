import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

ColumnLayout {
	id: footer
	anchors {
		left: parent.left
		right: parent.right
		leftMargin: !inPortrait ? drawer.width : 12
		rightMargin: !inPortrait ? 0 : 12
	}
	RowLayout {
		Layout.topMargin: 16
		Slider {
			Layout.fillWidth: true
			value: 0
		}
		Label {
			text: "0:00/0:00"
		}
	}
	RowLayout {
		Layout.bottomMargin: 6
		Item {
			Layout.fillWidth: true
		}
		ToolButton {
			icon.name: "media-playlist-shuffle"
			icon.source: icSrc("media-playlist-shuffle")
			checkable: true
		}
		ToolButton {
			icon.name: "media-playlist-repeat"
			icon.source: icSrc("media-playlist-repeat")
			checkable: true
		}
		ToolSeparator {
		}
		ToolButton {
			icon.name: "media-skip-backward"
			icon.source: icSrc("media-skip-backward")
		}
		ToolButton {
			icon.name: "media-playback-start"
			icon.source: icSrc("media-playback-start")
		}
		ToolButton {
			icon.name: "media-skip-forward"
			icon.source: icSrc("media-skip-forward")
		}
		ToolSeparator {
		}
		ToolButton {
			icon.name: "audio-volume-high"
			icon.source: icSrc("audio-volume-high")
		}
		ToolButton {
			icon.name: "overflow-menu"
			icon.source: icSrc("overflow-menu")
		}
		Item {
			Layout.fillWidth: true
		}
	}
}