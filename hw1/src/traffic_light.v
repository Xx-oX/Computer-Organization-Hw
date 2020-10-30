module traffic_light (clk,rst,pass,R,G,Y);

parameter ON = 1'b1;
parameter OFF = 1'b0;

input clk,rst,pass;
output reg R,G,Y;
reg [11:0]count;
reg isInitState;

always@(posedge clk, posedge rst) 
begin
	if(rst)
	begin
		R <= OFF;
		G <= ON;
		Y <= OFF;
		count <= 12'd1;
		isInitState <= ON; 
		//$write("==========rst==========\n");
	end
	else
	begin
		if(pass == ON)
		begin
			if(G == ON && isInitState == ON)
			begin	
				count <= count + 12'd1;
				//$write("==========skip==========\n");
			end
			else
			begin
				R <= OFF;
				G <= ON;
				Y <= OFF;
				count <= 12'd1;
				//$write("==========pass==========\n");
			end
		end
		else 
		begin
			if(count >= 12'd0 && count < 12'd1024)
			begin
				R <= OFF;
				G <= ON;
				Y <= OFF;
				isInitState <= ON;
				count <= count + 12'd1;
			end
			else if(count >= 12'd1024 && count < 12'd1152)
			begin
				R <= OFF;
				G <= OFF;
				Y <= OFF;
				isInitState <= OFF;
				count <= count + 12'd1;
			end
			else if(count >= 12'd1152 && count < 12'd1280)
			begin
				R <= OFF;
				G <= ON;
				Y <= OFF;
				isInitState <= OFF;
				count <= count + 12'd1;
			end
			else if(count >= 12'd1280 && count < 12'd1408)
			begin
				R <= OFF;
				G <= OFF;
				Y <= OFF;
				isInitState <= OFF;
				count <= count + 12'd1;
			end
			else if(count >= 12'd1408 && count < 12'd1536)			begin
				R <= OFF;
				G <= ON;
				Y <= OFF;
				isInitState <= OFF;
				count <= count + 12'd1;
			end
			else if(count >= 12'd1536 && count < 12'd2048)
			begin
				R <= OFF;
				G <= OFF;
				Y <= ON;
				isInitState <= OFF;
				count <= count + 12'd1;
			end
			else if(count >= 12'd2048 && count < 12'd3072)
			begin
				if(count == 12'd3071)
				begin
					count <= 12'd0;
				end
				else
				begin
					R <= ON;
					G <= OFF;
					Y <= OFF;
					isInitState <= OFF;
					count <= count + 12'd1;
				end
			end
			else
			begin
				count <= 12'd1;
			end
		end
	end
end
endmodule
