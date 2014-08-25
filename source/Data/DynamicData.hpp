#ifndef DynamicImage_HPP
#define DynamicImage_HPP

#include "Streamer.hpp"
#include "DataObject.hpp"
#include <vector>

namespace fast {

template <class T>
class DynamicData : public virtual DataObject {
    public:                                                     
        typedef SharedPointer<DynamicData<T> > pointer;
        static typename DynamicData<T>::pointer New() {                       
            DynamicData<T> * ptr = new DynamicData<T>();                  
            typename DynamicData<T>::pointer smartPtr(ptr);                   
            ptr->setPtr(smartPtr);                              
                                                                
            return smartPtr;                                    
        }                                                       
    private:                                                    
        WeakPointer<DynamicData<T> > mPtr;
        void setPtr(typename DynamicData<T>::pointer ptr) {                   
            mPtr = ptr;                                         
        }                                                       
    public:
        typename T::pointer getNextFrame();
        void addFrame(typename T::pointer frame);
        unsigned int getSize() const;
        ~DynamicData() {};
        void setStreamer(Streamer::pointer streamer);
        Streamer::pointer getStreamer();
        bool hasReachedEnd();
        typename T::pointer getCurrentFrame();
    private:

        WeakPointer<Streamer> mStreamer;

        // If the flag mKeepAllFrames is set to false, this vector will have
        // a max size of 1
        std::vector<typename T::pointer> mFrames;

        // Keep track of which frame is next, only used when mKeepAllFrames is
        // set to true
        unsigned long mCurrentFrame;



        boost::mutex mStreamMutex;

        bool mHasReachedEnd;
    protected:
        DynamicData();
        // TODO not implemented yet
        void free(ExecutionDevice::pointer device) {};
        void freeAll() {};
};


template <class T>
typename T::pointer DynamicData<T>::getNextFrame() {
    mStreamMutex.lock();

    // Check if no frames are available
    if(mFrames.size() == 0 || mFrames.size() <= mCurrentFrame) {
        if(mStreamer.lock()->hasReachedEnd()) {
            mStreamMutex.unlock();
            throw Exception("Streamer has reached the end.");
        } else {
            mStreamMutex.unlock();
            throw Exception("This exception should not have occured. There is a bug somewhere in which getNextFrame is called more than once for the same timestamp.");
        }
    }

    // Get the frame
    typename T::pointer ret;
    if(mStreamer.lock()->getStreamingMode() == STREAMING_MODE_STORE_ALL_FRAMES) {
        // Get the next one
        ret = mFrames[mCurrentFrame];
        mCurrentFrame++;
    } else {
        // Always get the one in front
        ret = mFrames[0];
    }

    // Remove the frame from collection if necessary
    if(mStreamer.lock()->getStreamingMode() == STREAMING_MODE_PROCESS_ALL_FRAMES) {
        // Remove the frame that was read, the one in the front
        mFrames.erase(mFrames.begin());
    }

    // Update modified timestamp so that a process object will read the next frame
    if(mStreamer.lock()->getStreamingMode() == STREAMING_MODE_STORE_ALL_FRAMES) {
        if(mCurrentFrame < mFrames.size())
            updateModifiedTimestamp();
    } else if(mStreamer.lock()->getStreamingMode() == STREAMING_MODE_PROCESS_ALL_FRAMES) {
        if(mFrames.size() > 0)
            updateModifiedTimestamp();
    }

    mStreamMutex.unlock();

    mBoundingBox = ret->getBoundingBox();

    return ret;
}

template <class T>
typename T::pointer DynamicData<T>::getCurrentFrame() {
    mStreamMutex.lock();
    if(mFrames.size() == 0 || mFrames.size() <= mCurrentFrame) {
        if(mStreamer.lock()->hasReachedEnd()) {
            mStreamMutex.unlock();
            throw Exception("Streamer has reached the end.");
        } else {
            mStreamMutex.unlock();
            throw Exception("This exception should not have occured. ");
        }
    }
    typename T::pointer ret = mFrames[mCurrentFrame];
    mStreamMutex.unlock();
    return ret;
}

template <class T>
void DynamicData<T>::addFrame(typename T::pointer frame) {
    if(!mStreamer.lock().isValid())
        throw Exception("A DynamicImage must have a streamer set before it can be used.");
    mStreamMutex.lock();
    updateModifiedTimestamp();
    if(mStreamer.lock()->getStreamingMode() == STREAMING_MODE_NEWEST_FRAME_ONLY) {
        mFrames.clear();
    }
    mFrames.push_back(frame);
    mStreamMutex.unlock();
}

template <class T>
DynamicData<T>::DynamicData() {
    mCurrentFrame = 0;
    mIsDynamicData = true;
    mHasReachedEnd = false;
    updateModifiedTimestamp();
}


template <class T>
unsigned int DynamicData<T>::getSize() const {
    return mFrames.size();
}

template <class T>
void DynamicData<T>::setStreamer(Streamer::pointer streamer) {
    mStreamer = streamer;
}

template <class T>
Streamer::pointer DynamicData<T>::getStreamer() {
    return mStreamer.lock();
}

template <class T>
bool DynamicData<T>::hasReachedEnd() {
    mStreamMutex.lock();
    if(!mStreamer.lock().isValid()) {
        mStreamMutex.unlock();
        throw Exception("A DynamicData must have a streamer set before it can be used.");
    }
    // Check if has reached end can be changed to true
    if(!mHasReachedEnd) {
        switch(mStreamer.lock()->getStreamingMode()) {
        case STREAMING_MODE_NEWEST_FRAME_ONLY:
            if(mStreamer.lock()->hasReachedEnd())
                mHasReachedEnd = true;
            break;
        case STREAMING_MODE_PROCESS_ALL_FRAMES:
            if(mStreamer.lock()->hasReachedEnd() && mFrames.size() == 0)
                mHasReachedEnd = true;
            break;
        case STREAMING_MODE_STORE_ALL_FRAMES:
            if(mStreamer.lock()->hasReachedEnd() && mCurrentFrame == mFrames.size())
                mHasReachedEnd = true;
            break;
        }
    }
    mStreamMutex.unlock();
    return mHasReachedEnd;
}

} // end namespace fast

#endif
