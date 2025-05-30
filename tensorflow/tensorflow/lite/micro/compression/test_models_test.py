# Copyright 2024 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import tensorflow as tf
from tflite_micro.tensorflow.lite.micro.compression import test_models
from tflite_micro.tensorflow.lite.python import schema_py_generated as tflite


class TestBuild(tf.test.TestCase):

  def setUp(self):
    self.flatbuffer = test_models.build(test_models.EXAMPLE_MODEL)

  def testNotDegenerate(self):
    model = tflite.ModelT.InitFromPackedBuf(self.flatbuffer, 0)
    self.assertEqual(model.operatorCodes[0].builtinCode,
                     tflite.BuiltinOperator.FULLY_CONNECTED)


if __name__ == "__main__":
  tf.test.main()
