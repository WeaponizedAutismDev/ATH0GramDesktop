// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2025
#include "image_view.h"

#include "ayu/features/messageshot/message_shot.h"
#include "styles/style_ayu_styles.h"

#include "ayu/utils/telegram_helpers.h"
#include "styles/style_chat.h"
#include "ui/painter.h"

ImageView::ImageView(QWidget *parent)
	: RpWidget(parent) {
}

void ImageView::setImage(const QImage &image) {
	if (this->image == image) {
		return;
	}

	const auto set = [=]
	{
		this->prevImage = this->image;
		this->image = image;

		const auto size = image.size() / style::DevicePixelRatio();
		setMinimumSize(size.grownBy(st::imageViewInnerPadding));

		if (this->animation.animating()) {
			this->animation.stop();
		}

		if (this->prevImage.isNull()) {
			update();
			return;
		}

		this->animation.start(
			[=]
			{
				update();
			},
			0.0,
			1.0,
			300,
			anim::easeInCubic);
	};

	if (this->image.isNull()) {
		set();
		return;
	}

	dispatchToMainThread(set, 100);
}

QImage ImageView::getImage() const {
	return image;
}

void ImageView::paintEvent(QPaintEvent *e) {
	Painter p(this);

	const auto brush = QBrush(AyuFeatures::MessageShot::makeDefaultBackgroundColor());

	QPainterPath path;
	path.addRoundedRect(rect(), st::roundRadiusLarge, st::roundRadiusLarge);

	p.fillPath(path, brush);

	if (!prevImage.isNull()) {
		const auto realRect = rect().marginsRemoved(st::imageViewInnerPadding);

		const auto resizedRect = QRect(
			(realRect.width() - prevImage.width() / style::DevicePixelRatio()) / 2 + st::imageViewInnerPadding.left(),
			(realRect.height() - prevImage.height() / style::DevicePixelRatio()) / 2 + st::imageViewInnerPadding.top(),
			prevImage.width() / style::DevicePixelRatio(),
			prevImage.height() / style::DevicePixelRatio());

		const auto opacity = 1.0 - animation.value(1.0);
		p.setOpacity(opacity);
		p.drawImage(resizedRect, prevImage);
		p.setOpacity(1.0);
	}

	if (!image.isNull()) {
		const auto realRect = rect().marginsRemoved(st::imageViewInnerPadding);

		const auto resizedRect = QRect(
			(realRect.width() - image.width() / style::DevicePixelRatio()) / 2 + st::imageViewInnerPadding.left(),
			(realRect.height() - image.height() / style::DevicePixelRatio()) / 2 + st::imageViewInnerPadding.top(),
			image.width() / style::DevicePixelRatio(),
			image.height() / style::DevicePixelRatio());

		const auto opacity = animation.value(1.0);
		p.setOpacity(opacity);
		p.drawImage(resizedRect, image);
		p.setOpacity(1.0);
	}
}

void ImageView::mousePressEvent(QMouseEvent *e) {
}
