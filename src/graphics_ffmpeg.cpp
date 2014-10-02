#include <iostream>

#include "graphics.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}


#ifdef __APPLE__
# include <OpenGL/gl.h>
#else
# include <GL/glew.h>
# include <GL/gl.h>
#endif


namespace iv { namespace graphics {

namespace {

    void ffmpeg_initialize() {
        static bool ffmpeg_initialized = false;
        if(!ffmpeg_initialized) {
            av_register_all();
            ffmpeg_initialized = true;
        }
    }

    class ByteStreamIOContext {
    private:
        ByteStreamIOContext(const ByteStreamIOContext  &);
        ByteStreamIOContext& operator = (const ByteStreamIOContext &);
    public:
        ByteStreamIOContext(ByteStream* stream_)
          : stream(stream_),
            buffer_size(1024 * 256),
            buffer(static_cast<unsigned char*>(av_malloc(buffer_size)))
        {
            ctx = avio_alloc_context(buffer, buffer_size, 0, this, &ByteStreamIOContext::read, NULL, &ByteStreamIOContext::seek);
        }

        ~ByteStreamIOContext() {
            av_free(ctx);
            av_free(buffer);
        }

        static int read(void *opaque, unsigned char *buf, int buf_size) {
            ByteStreamIOContext* h = static_cast<ByteStreamIOContext*>(opaque);
            return h->stream->read(buf, buf_size);
        }

        static int64_t seek(void *opaque, int64_t offset, int whence) {
            ByteStreamIOContext* h = static_cast<ByteStreamIOContext*>(opaque);
            if(whence == SEEK_SET) {
                h->stream->seek(ByteStream::BEGIN, offset);
                return h->stream->position();
            }
            if(whence == SEEK_CUR) {
                h->stream->seek(ByteStream::CURRENT, offset);
                return h->stream->position();
            }
            if(whence == SEEK_END) {
                h->stream->seek(ByteStream::END, offset);
                return h->stream->position();
            }

        }

        AVIOContext *get_avio() { return ctx; }

    private:
        int64_t position;
        ByteStream* stream;
        int buffer_size;
        unsigned char * buffer;
        AVIOContext * ctx;
    };

    class VideoSurface2D_ffmpeg : public VideoSurface2D {
    public:

        VideoSurface2D_ffmpeg(ByteStream* stream) {
            std::string error_info;

            ffmpeg_initialize();

            mIOContext = NULL;
            mFormatContext = NULL;
            mCodecContext = NULL;
            mCodec = NULL;
            mFrame = NULL;
            mFrameRGB = NULL;
            mOptionsDict = NULL;

            mIOContext = new ByteStreamIOContext(stream);

            mFormatContext = avformat_alloc_context();
            mFormatContext->pb = mIOContext->get_avio();

            avformat_open_input(&mFormatContext, "dummy", nullptr, nullptr);

            if(avformat_find_stream_info(mFormatContext, NULL) < 0) {
                _cleanup();
                throw std::invalid_argument("could not find stream information");
            }

            av_dump_format(mFormatContext, 0, "VideoSurface2D_ffmpeg(ByteStream*)", 0);

            mVideoStream = -1;
            for(int i = 0; i < mFormatContext->nb_streams; i++) {
                if(mFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
                    mVideoStream = i;
                    break;
                }
            }

            if(mVideoStream == -1) {
                _cleanup();
                throw std::invalid_argument("could not find video stream");
            }

            mCodecContext = mFormatContext->streams[mVideoStream]->codec;

            mCodec = avcodec_find_decoder(mCodecContext->codec_id);

            if(mCodec == NULL) {
                _cleanup();
                throw std::invalid_argument("could not find video decoder");
            }

            if(avcodec_open2(mCodecContext, mCodec, &mOptionsDict) < 0) {
                _cleanup();
                throw std::invalid_argument("could not open codec");
            }

            mFrame = av_frame_alloc();
            mFrameRGB = av_frame_alloc();

            int mNumBytes = avpicture_get_size(PIX_FMT_RGBA, mCodecContext->width, mCodecContext->height);
            mBuffer = (uint8_t *)av_malloc(mNumBytes * sizeof(uint8_t));

            sws_ctx = sws_getContext(
                mCodecContext->width,
                mCodecContext->height,
                mCodecContext->pix_fmt,
                mCodecContext->width,
                mCodecContext->height,
                PIX_FMT_RGBA, SWS_BILINEAR,
                NULL, NULL, NULL
            );

            // Assign appropriate parts of buffer to image planes in pFrameRGB
            // Note that pFrameRGB is an AVFrame, but AVFrame is a superset of AVPicture
            avpicture_fill((AVPicture*)mFrameRGB, mBuffer, PIX_FMT_RGBA, mCodecContext->width, mCodecContext->height);

//   while(av_read_frame(pFormatCtx, &packet)>=0) {
//     // Is this a packet from the video stream?
//     if(packet.stream_index==videoStream) {
//       // Decode video frame
//       avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,
//                &packet);

//       // Did we get a video frame?
//       if(frameFinished) {
//     // Convert the image from its native format to RGB
//         sws_scale
//         (
//             sws_ctx,
//             (uint8_t const * const *)pFrame->data,
//             pFrame->linesize,
//             0,
//             pCodecCtx->height,
//             pFrameRGB->data,
//             pFrameRGB->linesize
//         );

//     // Save the frame to disk
//     if(++i<=5)
//       SaveFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height,
//             i);
//       }
//     }

//     // Free the packet that was allocated by av_read_frame
//     av_free_packet(&packet);
//   }

//   // Free the RGB image
//   av_free(buffer);
//   av_free(pFrameRGB);

//   // Free the YUV frame
//   av_free(pFrame);

//   // Close the codec
//   avcodec_close(pCodecCtx);

//   // Close the video file
//   avformat_close_input(&pFormatCtx);
        }

        void _cleanup() {
            if(mFormatContext) { av_free(mFormatContext); mFormatContext = NULL; }
            if(mFrame) { av_free(mFrame); mFrame = NULL; }
            if(mIOContext) { delete mIOContext; mIOContext = NULL; }
        }

        virtual ~VideoSurface2D_ffmpeg() {
            _cleanup();
        }

        virtual int width() const {
            return mCodecContext->width;
        }

        virtual int height() const {
            return mCodecContext->height;
        }

        virtual int frameCount() {
            return 0;
        }

        virtual void seek(int frame_index) {

        }

        virtual bool nextFrame() {
            while(av_read_frame(mFormatContext, &mPacket) >= 0) {
                if(mPacket.stream_index == mVideoStream) {
                    int frameFinished;
                    avcodec_decode_video2(mCodecContext, mFrame, &frameFinished, &mPacket);
                    if(frameFinished) {
                        sws_scale(
                            sws_ctx,
                            (uint8_t const * const *)mFrame->data,
                            mFrame->linesize,
                            0,
                            mCodecContext->height,
                            mFrameRGB->data,
                            mFrameRGB->linesize
                        );
                        return true;
                    }
                }
            }
            return false;
        }

        virtual double fps() {
            return 0;
        }

        virtual const void* pixels() const {
            return mBuffer;
        }

        ByteStreamIOContext* mIOContext;

        AVFormatContext *mFormatContext;
        AVCodecContext  *mCodecContext;
        AVCodec         *mCodec;
        AVFrame         *mFrame;
        AVFrame         *mFrameRGB;
        AVDictionary    *mOptionsDict;
        AVPacket        mPacket;
        int             mVideoStream;
        int             frameFinished;
        int             mNumBytes;
        uint8_t         *mBuffer;
        struct SwsContext *sws_ctx;
    };

}

VideoSurface2D* VideoSurface2D::FromStream(ByteStream* stream) {
    try {
        return new VideoSurface2D_ffmpeg(stream);
    } catch(std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

} }
