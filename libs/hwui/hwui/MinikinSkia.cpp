/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "MinikinSkia.h"

#include <SkPaint.h>
#include <SkTypeface.h>
#include <cutils/log.h>

namespace android {

MinikinFontSkia::MinikinFontSkia(sk_sp<SkTypeface> typeface, const void* fontData, size_t fontSize,
        int ttcIndex) :
    minikin::MinikinFont(typeface->uniqueID()), mTypeface(std::move(typeface)), mFontData(fontData),
    mFontSize(fontSize), mTtcIndex(ttcIndex) {
}

static void MinikinFontSkia_SetSkiaPaint(const minikin::MinikinFont* font, SkPaint* skPaint,
        const minikin::MinikinPaint& paint) {
    skPaint->setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    skPaint->setTextSize(paint.size);
    skPaint->setTextScaleX(paint.scaleX);
    skPaint->setTextSkewX(paint.skewX);
    MinikinFontSkia::unpackPaintFlags(skPaint, paint.paintFlags);
    // Apply font fakery on top of user-supplied flags.
    MinikinFontSkia::populateSkPaint(skPaint, font, paint.fakery);
}

float MinikinFontSkia::GetHorizontalAdvance(uint32_t glyph_id,
        const minikin::MinikinPaint &paint) const {
    SkPaint skPaint;
    uint16_t glyph16 = glyph_id;
    SkScalar skWidth;
    MinikinFontSkia_SetSkiaPaint(this, &skPaint, paint);
    skPaint.getTextWidths(&glyph16, sizeof(glyph16), &skWidth, NULL);
#ifdef VERBOSE
    ALOGD("width for typeface %d glyph %d = %f", mTypeface->uniqueID(), glyph_id, skWidth);
#endif
    return skWidth;
}

void MinikinFontSkia::GetBounds(minikin::MinikinRect* bounds, uint32_t glyph_id,
        const minikin::MinikinPaint& paint) const {
    SkPaint skPaint;
    uint16_t glyph16 = glyph_id;
    SkRect skBounds;
    MinikinFontSkia_SetSkiaPaint(this, &skPaint, paint);
    skPaint.getTextWidths(&glyph16, sizeof(glyph16), NULL, &skBounds);
    bounds->mLeft = skBounds.fLeft;
    bounds->mTop = skBounds.fTop;
    bounds->mRight = skBounds.fRight;
    bounds->mBottom = skBounds.fBottom;
}

SkTypeface *MinikinFontSkia::GetSkTypeface() const {
    return mTypeface.get();
}

sk_sp<SkTypeface> MinikinFontSkia::RefSkTypeface() const {
    return mTypeface;
}

const void* MinikinFontSkia::GetFontData() const {
    return mFontData;
}

size_t MinikinFontSkia::GetFontSize() const {
    return mFontSize;
}

int MinikinFontSkia::GetFontIndex() const {
    return mTtcIndex;
}

uint32_t MinikinFontSkia::packPaintFlags(const SkPaint* paint) {
    uint32_t flags = paint->getFlags();
    SkPaint::Hinting hinting = paint->getHinting();
    // select only flags that might affect text layout
    flags &= (SkPaint::kAntiAlias_Flag | SkPaint::kFakeBoldText_Flag | SkPaint::kLinearText_Flag |
            SkPaint::kSubpixelText_Flag | SkPaint::kDevKernText_Flag |
            SkPaint::kEmbeddedBitmapText_Flag | SkPaint::kAutoHinting_Flag |
            SkPaint::kVerticalText_Flag);
    flags |= (hinting << 16);
    return flags;
}

void MinikinFontSkia::unpackPaintFlags(SkPaint* paint, uint32_t paintFlags) {
    paint->setFlags(paintFlags & SkPaint::kAllFlags);
    paint->setHinting(static_cast<SkPaint::Hinting>(paintFlags >> 16));
}

void MinikinFontSkia::populateSkPaint(SkPaint* paint, const MinikinFont* font,
        minikin::FontFakery fakery) {
    paint->setTypeface(reinterpret_cast<const MinikinFontSkia*>(font)->RefSkTypeface());
    paint->setFakeBoldText(paint->isFakeBoldText() || fakery.isFakeBold());
    if (fakery.isFakeItalic()) {
        paint->setTextSkewX(paint->getTextSkewX() - 0.25f);
    }
}

}
