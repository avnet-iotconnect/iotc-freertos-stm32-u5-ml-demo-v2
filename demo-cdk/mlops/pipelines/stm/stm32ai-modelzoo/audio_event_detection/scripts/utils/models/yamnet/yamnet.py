# /*---------------------------------------------------------------------------------------------
#  * Copyright (c) 2022 STMicroelectronics.
#  * All rights reserved.
#  *
#  * This software is licensed under terms that can be found in the LICENSE file in
#  * the root directory of this software component.
#  * If no LICENSE file comes with this software, it is provided AS-IS.
#  *--------------------------------------------------------------------------------------------*/

import boto3
import tensorflow as tf
from pathlib import Path
from ..model_utils import add_head
from keras import layers
from keras import regularizers

def check_s3_object_exists(bucket_name, object_key):
    s3 = boto3.client('s3')

    try:
        # Attempt to retrieve the object's metadata
        s3.head_object(Bucket=bucket_name, Key=object_key)
        print(f"Object {object_key} exists in bucket {bucket_name}.")
        return True
    except ClientError as e:
        # Check the specific error code
        if e.response['Error']['Code'] == "404":
            print(f"Object {object_key} does not exist in bucket {bucket_name}.")
            return False
        else:
            # Something else went wrong; log and re-raise
            print(f"Error checking object: {e}")
            raise

def download_model_from_s3(bucket_name, object_key, local_path):
    s3 = boto3.client('s3')
    try:
        s3.download_file(bucket_name, object_key, local_path)
        print(f"Downloaded model from s3://{bucket_name}/{object_key} to {local_path}")
    except Exception as e:
        print(f"Error downloading model: {e}")
        raise

def get_pretrained_model(cfg):
    # Load model


    # !!!!Uncomment this code to be able to retrain existing model
    #
    # if check_s3_object_exists('retrain-model', 'best_model.h5'):
    #     local_download_path = Path(Path(__file__).parent.resolve(), 'downloaded_model.h5')

    #     download_model_from_s3('retrain-model', 'best_model.h5', str(local_download_path))
    #     yamnet_backbone = tf.keras.models.load_model(local_download_path)
    # else:
    #     yamnet_backbone = tf.keras.models.load_model(Path(Path(__file__).parent.resolve(),
    #                      'yamnet_{}_f32.h5'.format(str(cfg.model.model_type.embedding_size))))

    


    yamnet_backbone = tf.keras.models.load_model(Path(Path(__file__).parent.resolve(),
                         'yamnet_{}_f32.h5'.format(str(cfg.model.model_type.embedding_size))))
    print("Backbone layers")
    print(yamnet_backbone.layers)
    # Add permutation layer
    inp = layers.Input(shape=(64, 96, 1))
    x = layers.Permute((2, 1, 3))(inp)
    out = yamnet_backbone(x)
    permuted_backbone = tf.keras.models.Model(inputs=inp, outputs=out)

    # Add head
    n_classes = len(cfg.dataset.class_names)
    if cfg.dataset.use_other_class:
        n_classes += 1
        
    if cfg.model.multi_label:
        activation = 'sigmoïd'
    else:
        activation ='softmax'

    yamnet = add_head(backbone=permuted_backbone,
                         n_classes=n_classes,
                         trainable_backbone=cfg.model.fine_tune,
                         add_flatten=False,
                         functional=True,
                         activation=activation,
                         kernel_regularizer=regularizers.L2(0),
                         activity_regularizer=regularizers.L1(0),
                         dropout=cfg.model.dropout)
                         
    return yamnet