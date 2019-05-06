# NeuralNetwork-JumpingGame

Simple Jumping game using Neural Network

Video Showcase: https://www.youtube.com/watch?v=iuogHNSpLgY

1) Generate jumping samples from random data
    * Multi-core option available for faster generation of data
2) Preprocess data
    * Remove very similar data
3) Train the neural network
    * Inputs
      a) Distance from the missile to character
      b) Speed of the missile
      c) Vertical position of the missile
    * Output
      a) Value to jump or not (-1 to 1), jumps when value is >= 0.9 
    * Targeted Mean Square Error(MSE) of 0.001
    * Maximum Epoch(Number of times the data is passed forward and backward into the network): 100000
4) See how the model plays out (Accelerate the time to see whether it can pass missiles of all different speeds)
