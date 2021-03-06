/*
Edit 22-01-2020: Got Splendid results with elu+lin activation with normalization
Edit 03-02-2020: Added error_threshold option to stop training
                 Added momentum functionality to boost learning (from 1861 epochs -> 1625 epochs)
*/
#include <iostream>
#include<vector>
#include <random>
#include <math.h>
#include <time.h> //FOR TIMING

#define PI 3.141592653589793238463

#define N
//#define epsilon 0.05
#define alpha 1         //AS PER TENSORFLOW DOCS
#define epoch 5000
#define momentum 0.09
#define error_threshold 1.e-005

using namespace std;
extern "C" FILE *popen(const char *command, const char *mode);

///SIGMOID ACTIVATION DEFINITIONS
double sigmoid(double x) { return 1.0f / (1.0f + exp(-x)); }
double dsigmoid(double x) { return x * (1.0f - x); }

///TANH ACTIVATION DEFINITIONS
double tanh(double x) { return (exp(x)-exp(-x))/(exp(x)+exp(-x)) ;}
double dtanh(double x) {return 1.0f - x*x ;}

///ELU ACTIVATION DEFINITIONS
double elu(double x) { if(x>0)  return x;
                       else return alpha*(exp(x)-1.0);
                     }
double delu(double x) { if(x>0) return 1.0f;
                       else return alpha*exp(x);
                      }

///LINEAR ACTIVATION DEFINITIONS
double lin(double x) { return x;}
double dlin(double x) { return 1.0f;}

///WEIGHT INITIALIZER
double init_weight() { int temp = (rand()%10);cout<<endl<<temp;
                        if(temp<6) return -1;
                        else return 0;}

double MAXX = -9999999999999999; //maximum value of input example
static const int numInputs = 3;
static const int numHiddenNodes = 9;
static const int numOutputs = 1;
static const int numTrainingSets = 8;    ///Train Set = 8
static const int numTestSets = 16;    ///Test Set = 16
const double lr = 0.05f;

double hiddenLayer[numHiddenNodes];
double outputLayer[numOutputs];

double hiddenLayerBias[numHiddenNodes];                     ///BIASES OF HIDDEN LAYER (c)
double outputLayerBias[numOutputs];                         ///BIASES OF OUTPUT LAYER (b)

double hiddenWeights[numInputs][numHiddenNodes];            ///WEIGHTS OF HIDDEN LAYER (W)
double outputWeights[numHiddenNodes][numOutputs];           ///WEIGHTS OF OUTPUT LAYER (V)

double del_hiddenWeights[numInputs][numHiddenNodes] = {0};
double del_outputWeights[numHiddenNodes][numOutputs] = {0};

//double training_inputs[numTrainingSets][numInputs];
//double training_outputs[numTrainingSets][numOutputs];

double training_inputs[numTrainingSets][numInputs] = { {255,255,255},
                                                       {255,218,185},
                                                       {245,255,250},
                                                       {230,230,250},
                                                       {0,255,0},
                                                       {0,100,0},
                                                       {46,139,87},
                                                       {127,255,0}
                                                     };

 double test_inputs[numTestSets][numInputs] =        { {0,0,255},
                                                       {135,206,235},
                                                       {175,238,238},
                                                       {127,255,212},
                                                       {255,0,0},
                                                       {255,69,0},
                                                       {255,127,0},
                                                       {255,165,0},
                                                       {0,0,0},
                                                       {105,105,105},
                                                       {112,112,112},
                                                       {169,169,169},
                                                       {155,48,255},
                                                       {139,35,35},
                                                       {205,51,51},
                                                       {255,246,143},
                                                     };

double training_outputs[numTrainingSets][numOutputs] ={ {765},
                                                        {658},
                                                        {750},
                                                        {710},
                                                        {255},
                                                        {100},
                                                        {272},
                                                        {382}
                                                      };

double test_outputs[numTestSets][numOutputs] =       {  {255},
                                                        {576},
                                                        {651},
                                                        {594},
                                                        {255},
                                                        {324},
                                                        {382},
                                                        {420},
                                                        {0},
                                                        {315},
                                                        {336},
                                                        {507},
                                                        {458},
                                                        {209},
                                                        {307},
                                                        {664}
                                                       };

void shuffle(int *array, size_t n)
{
    if (n > 1) //If no. of training examples > 1
    {
        size_t i;
        for (i = 0; i < n - 1; i++)
        {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

double predict(double test_sample[])
{
    for (int j=0; j<numHiddenNodes; j++)
    {
        double activation=hiddenLayerBias[j];
        for (int k=0; k<numInputs; k++)
        {
            activation+=test_sample[k]*hiddenWeights[k][j];
        }
        hiddenLayer[j] = elu(activation);
    }

    for (int j=0; j<numOutputs; j++)
    {
        double activation=outputLayerBias[j];
        for (int k=0; k<numHiddenNodes; k++)
        {
            activation+=hiddenLayer[k]*outputWeights[k][j];
        }
        outputLayer[j] = lin(activation);
    }
    //std::cout<<outputLayer[0]<<"\n";
    return outputLayer[0];
    //std::cout << "Input:" << training_inputs[i][0] << " " << training_inputs[i][1] << "    Output:" << outputLayer[0] << "    Expected Output: " << training_outputs[i][0] << "\n";
}

int main(int argc, const char * argv[])
{
    ///TRAINING DATA GENERATION
    clock_t tstart = clock();
    for (int i = 0; i < numTrainingSets; i++)
    {
		///APPLYING CHROMATICITY CALCULATION
		training_inputs[i][0] = training_inputs[i][0]*(-0.14282) + training_inputs[i][1]*(1.54924) + training_inputs[i][2]*(-0.95641);
		training_inputs[i][1] = training_inputs[i][0]*(-0.32466) + training_inputs[i][1]*(1.57837) + training_inputs[i][2]*(-0.73191);
		training_inputs[i][2] = training_inputs[i][0]*(-0.68202) + training_inputs[i][1]*(0.77073) + training_inputs[i][2]*(0.56332);

		training_outputs[i][0] = training_inputs[i][0]+training_inputs[i][1]+training_inputs[i][2];

		/***************************Try Avoiding Edits In This part*******************************/
        ///FINDING NORMALIZING FACTOR
		for(int m=0; m<numInputs; ++m)
            if(MAXX < training_inputs[i][m])
                MAXX = training_inputs[i][m];
        for(int m=0; m<numOutputs; ++m)
            if(MAXX < training_outputs[i][m])
                MAXX = training_outputs[i][m];
    }
    cout<< "\nFound Normalizing Factor = "<<MAXX;

	///NORMALIZING
	for (int i = 0; i < numTrainingSets; i++)
	{
        for(int m=0; m<numInputs; ++m)
            training_inputs[i][m] /= 1.0f*MAXX*numInputs;

        for(int m=0; m<numOutputs; ++m)
            training_outputs[i][m] /= 1.0f*MAXX*numInputs;

        cout<<"In: "<<training_inputs[i][0]<< ", " <<training_inputs[i][1]<<", "<<training_inputs[i][2]<<"       out: "<<training_outputs[i][0]<<endl;
	}
    ///WEIGHT & BIAS INITIALIZATION
    for (int i=0; i<numInputs; i++) {
        for (int j=0; j<numHiddenNodes; j++) {
            hiddenWeights[i][j] = init_weight();
        }
    }
    for (int i=0; i<numHiddenNodes; i++) {
        hiddenLayerBias[i] = init_weight();
        for (int j=0; j<numOutputs; j++) {
            outputWeights[i][j] = init_weight();
        }
    }
    for (int i=0; i<numOutputs; i++) {
        //outputLayerBias[i] = init_weight();
        outputLayerBias[i] = 0;
    }

    ///FOR INDEX SHUFFLING
    int trainingSetOrder[numTrainingSets];
    for(int j=0; j<numTrainingSets; ++j)
        trainingSetOrder[j] = j;


    ///TRAINING
    //std::cout<<"start train\n";
    vector<double> performance, epo; ///STORE MSE, EPOCH
    double MSE = 0;
    int n=0;
    for (n=0; (n < epoch); n++)
    {
        shuffle(trainingSetOrder,numTrainingSets);
        std::cout<<"\nepoch :"<<n << "\tMSE: "<<MSE;
        for (int x=0; x<numTrainingSets; x++)
        {
            int i = trainingSetOrder[x];
            //std::cout<<"Training Set :"<<x<<"\n";
            /// Forward pass
            for (int j=0; j<numHiddenNodes; j++)
            {
                double activation=hiddenLayerBias[j];
                //std::cout<<"Training Set :"<<x<<"\n";
                 for (int k=0; k<numInputs; k++) {
                    activation+=training_inputs[i][k]*hiddenWeights[k][j];
                }
                hiddenLayer[j] = elu(activation);
            }

            for (int j=0; j<numOutputs; j++) {
                double activation=outputLayerBias[j];
                for (int k=0; k<numHiddenNodes; k++)
                {
                    activation+=hiddenLayer[k]*outputWeights[k][j];
                }
                outputLayer[j] = lin(activation);
            }

            //std::cout << "Input:" << training_inputs[x][0] << " " << "    Output:" << outputLayer[0] << "    Expected Output: " << training_outputs[x][0] << "\n";
            for(int k=0; k<numOutputs; ++k)
                MSE += (1.0f/numOutputs)*pow( training_outputs[i][k] - outputLayer[k], 2);

           /// Backprop
           ///   For V
            double deltaOutput[numOutputs];
            for (int j=0; j<numOutputs; j++) {
                double errorOutput = (training_outputs[i][j]-outputLayer[j]);
                deltaOutput[j] = errorOutput*dlin(outputLayer[j]);
            }

            ///   For W
            double deltaHidden[numHiddenNodes];
            for (int j=0; j<numHiddenNodes; j++) {
                double errorHidden = 0.0f;
                for(int k=0; k<numOutputs; k++) {
                    errorHidden+=deltaOutput[k]*outputWeights[j][k];
                }
                deltaHidden[j] = errorHidden*delu(hiddenLayer[j]);
            }

            ///Updation
            ///   For V and b
            for (int j=0; j<numOutputs; j++) {
                //b
                outputLayerBias[j] += deltaOutput[j]*lr;
                for (int k=0; k<numHiddenNodes; k++)
                {
                    del_hiddenWeights[k][j] = (hiddenLayer[k]*deltaOutput[j]*lr) + momentum*del_hiddenWeights[k][j];
                    outputWeights[k][j]+= del_hiddenWeights[k][j];
                }
            }

            ///   For W and c
            for (int j=0; j<numHiddenNodes; j++) {
                //c
                hiddenLayerBias[j] += deltaHidden[j]*lr;
                //W
                for(int k=0; k<numInputs; k++) {
                  del_hiddenWeights[k][j] = (training_inputs[i][k]*deltaHidden[j]*lr) + momentum*del_hiddenWeights[k][j];
                  hiddenWeights[k][j]+= del_hiddenWeights[k][j];
                }
            }
        }
        //Averaging the MSE
        MSE /= 1.0f*numTrainingSets;
        //cout<< "  MSE: "<< MSE<<endl;
        ///Steps to PLOT PERFORMANCE PER EPOCH
        performance.push_back(MSE);

        epo.push_back(n);

        ///Break if error drops down below threshold
        if(MSE < error_threshold)
            break;
    }
    //cout<<endl<<"TRAINING TIME: "<<(double)(clock() - tstart)/CLOCKS_PER_SEC)<<" ms\n";
    printf("TRAINING TIME: %.2fs\n", (double)(clock() - tstart)/CLOCKS_PER_SEC);

    //HERE : ;
    // Print weights
    std::cout << "Final Hidden Weights\n[ ";
    for (int j=0; j<numHiddenNodes; j++) {
        std::cout << "[ ";
        for(int k=0; k<numInputs; k++) {
            std::cout << hiddenWeights[k][j] << " ";
        }
        std::cout << "] ";
    }
    std::cout << "]\n";

    std::cout << "Final Hidden Biases\n[ ";
    for (int j=0; j<numHiddenNodes; j++) {
        std::cout << hiddenLayerBias[j] << " ";

    }
    std::cout << "]\n";
    std::cout << "Final Output Weights";
    for (int j=0; j<numOutputs; j++) {
        std::cout << "[ ";
        for (int k=0; k<numHiddenNodes; k++) {
            std::cout << outputWeights[k][j] << " ";
        }
        std::cout << "]\n";
    }
    std::cout << "Final Output Biases\n[ ";
    for (int j=0; j<numOutputs; j++) {
        std::cout << outputLayerBias[j] << " ";
    }
    std::cout << "]\n";

    ///Plot the results
	vector<double> x;
	vector<double> y1, y2;

	for (int i = 0; i < numTestSets; i++)
    {
		x.push_back(i);

        ///APPLYING CHROMATICITY CALCULATION
		test_inputs[i][0] = test_inputs[i][0]*(-0.14282) + test_inputs[i][1]*(1.54924) + test_inputs[i][2]*(-0.95641);
		test_inputs[i][1] = test_inputs[i][0]*(-0.32466) + test_inputs[i][1]*(1.57837) + test_inputs[i][2]*(-0.73191);
		test_inputs[i][2] = test_inputs[i][0]*(-0.68202) + test_inputs[i][1]*(0.77073) + test_inputs[i][2]*(0.56332);

		test_outputs[i][0] = test_inputs[i][0]+test_inputs[i][1]+test_inputs[i][2];

		//test_input[i][0] = (rand()%MAXX);
		//test_input[i][1] = (rand()%MAXX);
		y1.push_back(test_outputs[i][0]);           //Expected

		test_inputs[i][0] /= MAXX*numInputs;
		test_inputs[i][1] /= MAXX*numInputs;
		test_inputs[i][2] /= MAXX*numInputs;

		y2.push_back(MAXX*numInputs*predict(test_inputs[i]));       //Predicted
		cout<< "\nExpected: "<<test_outputs[i][0] << "\tGot: "<<MAXX*numInputs*predict(test_inputs[i]);

	}
    cout<<"\n Final MSE: "<<MSE<<" at epoch: "<<n-1;
	FILE * gp = popen("gnuplot", "w");
	fprintf(gp, "set terminal wxt size 600,400 \n");
	fprintf(gp, "set grid \n");
	fprintf(gp, "set title '%s' \n", "RGB to R^ G^ B^");
	fprintf(gp, "set style line 1 lt 3 pt 7 ps 0.5 lc rgb 'green' lw 1 \n");
	fprintf(gp, "set style line 2 lt 3 pt 7 ps 0.5 lc rgb 'red' lw 1 \n");
	fprintf(gp, "plot '-' w p ls 1, '-' w p ls 2 \n");


	///Exact f(x) = addition -> Green Graph
	for (int k = 0; k < x.size(); k++) {
		fprintf(gp, "%f %f \n", x[k], y1[k]);
	}
	fprintf(gp, "e\n");

	///Neural Network Approximate -> Red Graph
	for (int k = 0; k < x.size(); k++) {
		fprintf(gp, "%f %f \n", x[k], y2[k]);
	}
	fprintf(gp, "e\n");

	fflush(gp);

	///FILE POINTER FOR SECOND PLOT (PERFORMANCE GRAPH)
    FILE * gp1 = popen("gnuplot", "w");
	fprintf(gp1, "set terminal wxt size 600,400 \n");
	fprintf(gp1, "set grid \n");
	fprintf(gp1, "set title '%s' \n", "Performance");
	fprintf(gp1, "set style line 1 lt 3 pt 7 ps 0.1 lc rgb 'green' lw 1 \n");
	fprintf(gp1, "set style line 2 lt 3 pt 7 ps 0.1 lc rgb 'red' lw 1 \n");
	fprintf(gp1, "plot '-' w p ls 1 \n");

    for (int k = 0; k < epo.size(); k++) {
		fprintf(gp1, "%f %f \n", epo[k], performance[k]);
	}
	fprintf(gp1, "e\n");

	fflush(gp1);

	system("pause");
	//pclose(gp);

    return 0;
}

