package com.wzq.ffmpegdemo.widget;

import android.animation.ValueAnimator;
import android.content.Context;
import android.content.res.Configuration;
import android.support.annotation.IntDef;
import android.util.AttributeSet;
import android.view.View;
import android.view.animation.LinearInterpolator;
import android.widget.RelativeLayout;

/**
 * Created by windows on 2017/10/13.
 */

public class ControlLayout extends RelativeLayout {
    ValueAnimator animator;

    public ControlLayout(Context context) {
        this(context, null);
    }

    public ControlLayout(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public ControlLayout(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }


    public void setVisibleBar(){
        if (getVisibility() == VISIBLE) {
            animator = new ValueAnimator();
            animator.setInterpolator(new LinearInterpolator());
            animator.setFloatValues(100, 0);
            animator.setDuration(7000);
            animator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
                @Override
                public void onAnimationUpdate(ValueAnimator valueAnimator) {
                    setAlpha((float) valueAnimator.getAnimatedValue());
                    if ((float) valueAnimator.getAnimatedValue() <= 0)
                        setVisibility(GONE);
                }
            });
            animator.start();
        }
    }

    @Override
    public void setVisibility(int visibility) {
        super.setVisibility(visibility);
        if (animator != null && animator.isRunning())
            animator.cancel();
    }


}
